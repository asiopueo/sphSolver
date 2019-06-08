
/************************************************
 * rtfluid-rewrite for Linux                    *
 * Martin Lippl                                 *
 * CLL: Cell-linked lists						*
 ************************************************/

// Standard includes
#include <memory.h>
#include <cmath>
#include <stdio.h>
#include <vector>
#include <assert.h>	
#include <iostream>
#include <string>
using namespace std;

// GLEW
#define GLEW_STATIC
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

// GLM
#define GLM_FORCE_RADIANS   // Care about this later.
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
using namespace glm;

// SOIL
#include <SOIL/SOIL.h>

// FreeType
#include <ft2build.h>
#include FT_FREETYPE_H

// Project-specific includes
#include "memory.h"
#include "common.h"
#include "physics.h"
#include "loader.h"
//#include "marchingcubes.h"

// Model data
#include "modeldata.h"

// Definition of constants
#define SMOOTHING_LENGTH 0.05f  // Ein wenig länger als der initiale Teilchenabstand.
#define SEARCH_RADIUS (SMOOTHING_LENGTH)
#define CUBE_LEN_X 10
#define CUBE_LEN_Y 10
#define CUBE_LEN_Z 10
#define SCAL_LEN 0.1f
#define N_PARTICLES (CUBE_LEN_X*CUBE_LEN_Y*CUBE_LEN_Z)
#define TIME_STEP 0.0001f   // TIME_STEP has influence on the stability of the program.
#define EPSILON 0.05f
#define VISCOSITY 0.1f
#define STIFF 1.0f
#define MASS 0.1f

#define ISO_THRESHOLD 700.0f
#define ISO_RADIUS 0.0115f
#define MC_GRID_LEN 0.005f

const float PI = 3.1415926535f;

#define RANDF() 0.2f * ((rand()/(GLfloat)RAND_MAX - 0.5f))



/***********************
 * Global variables (better put them into "common.h" later)
 ***********************/

GLFWwindow* window;

float CurrentTime;
float PastTime = 0.0f;

float azimuth = 0.0f;	// Note that glm interpretes the angles as rad.
float zenith = PI/2.0F;

// 'Dreibein' of the Camera
vec3 DiVector;
vec3 RiVector;
vec3 UpVector;

vec3 Position;

mat4 ModelMatrix;
mat4 ViewMatrix;
mat4 ProjectionMatrix;
mat4 VP_matrix, MVP_matrix;
int debug_counter=0;

// SPH simulation instance
sph_struc sph_instance;
grid_struc grid_instance;
neighbor_struc nbr_list;
//render_struc render_instance;
vec3* vertexdata;

// Texture
GLuint cubemap_tex;

// FreeType
FT_Library library;
FT_Face face;


/*************************/



GLfloat wrap_angle( float angle )
{
	float twoPi = 2.0 * PI;
	return angle - twoPi * floor( angle / 3 );
}



void compute_matrices_from_inputs()
{
	double xPos, yPos;
	vec3 MovementDirection;

	CurrentTime = glfwGetTime();

	glfwGetCursorPos(window, &xPos, &yPos);


	azimuth -= 0.0001f*float(xPos-512);
	zenith += 0.0001f*float(yPos-384);

	zenith = clamp(0.0f, zenith, PI);

	// Für die Konventionen des Koordinatensystems fehlt mir noch das nötige Verständnis.
	DiVector = vec3(sin(azimuth) * sin(zenith), 
								   cos(zenith), 
					cos(azimuth) * sin(zenith));
	RiVector = vec3(sin(azimuth - PI/2.0f), 
					0.0f, 
					cos(azimuth - PI/2.0f));
	UpVector = cross(RiVector, DiVector);

	// WASD-movements
	if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		Position += mat3(0.01f)*DiVector;

	if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		Position -= mat3(0.01f)*DiVector;

	if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		Position += mat3(0.01f)*RiVector;

	if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		Position -= mat3(0.01f)*RiVector;

	// Altitude control
	if(glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
		Position += mat3(0.01f)*vec3(0.0f, 1.0f, 0.0f);

	if(glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
		Position -= mat3(0.01f)*vec3(0.0f, 1.0f, 0.0f);
}



void render_text(GLint freetype_shdrs, const char *text, float x, float y, float sx, float sy)
{
	const char *p;

	FT_GlyphSlot g = face->glyph;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glUseProgram(freetype_shdrs);

	/* Create a texture that will be used to hold one "glyph" */
	GLuint tex, text_vbo, text_vao;
	glGenVertexArrays(1,&text_vao);
	
	glActiveTexture(GL_TEXTURE1);
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);

	GLuint uniform_tex;
	uniform_tex = glGetUniformLocation(freetype_shdrs, "text");
	glUniform1i(uniform_tex, 1);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	glBindVertexArray(text_vao);
	glEnableVertexAttribArray(0);
	glGenBuffers(1,&text_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, text_vbo);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);

	for(p = text; *p; p++) 
	{
		if(FT_Load_Char(face, *p, FT_LOAD_RENDER))
			continue;

		glTexImage2D( GL_TEXTURE_2D, 0, GL_RED, g->bitmap.width, g->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE, g->bitmap.buffer);

		float x2 = x + g->bitmap_left * sx;
		float y2 = -y - g->bitmap_top * sy;
		float w = g->bitmap.width * sx;
		float h = g->bitmap.rows * sy;
		
		// First two components are screen coordinates, the last two are uv-coordinates.
		GLfloat box[4][4] = {
			{x2,     -y2    , 0.0f, 0.0f},
			{x2 + w, -y2    , 1.0f, 0.0f},
			{x2,     -y2 - h, 0.0f, 1.0f},
			{x2 + w, -y2 - h, 1.0f, 1.0f}
		};
		
		glBufferData(GL_ARRAY_BUFFER, sizeof(box), box, GL_STATIC_DRAW);
		//glDrawArrays(GL_TRIANGLES, 0, 3);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		//glDrawArrays(GL_POINTS, 0, 4);

		x += (g->advance.x/64.0f) * sx;
		y += (g->advance.y/64.0f) * sy;
	}

	glDisable(GL_BLEND);
	glDisableVertexAttribArray(0);
	glBindVertexArray(0);
	glDeleteTextures(1, &tex);
}



// Initialize the states of particles
void init_sph(void)
{
	int i;

	vec3 pos[N_PARTICLES];
	vec3 vel[N_PARTICLES];

	mat4 rot;
	mat4 rotview;

	for (int x = 0; x < CUBE_LEN_X; x++)
		for (int y = 0; y < CUBE_LEN_Y; y++)
			for (int z = 0; z < CUBE_LEN_Z; z++)
			{
				i = x + y*CUBE_LEN_X + z*CUBE_LEN_X*CUBE_LEN_Y;
				pos[i] = vec3(SCAL_LEN*(x + RANDF() - CUBE_LEN_X/2), SCAL_LEN*(y + RANDF() - CUBE_LEN_Y/2), SCAL_LEN*(z + RANDF() - CUBE_LEN_Z/2));
				vel[i] = vec3(0.0f, 0.0f, 0.0f);
			}

	create_sph_instance(&sph_instance, N_PARTICLES, pos, vel, SMOOTHING_LENGTH, VISCOSITY, MASS, STIFF, SEARCH_RADIUS, TIME_STEP);
}




// Physical calculations are done here!
void elapse(void)
{
	elapse_water(&sph_instance, &grid_instance, &nbr_list);
}




GLuint loadCubemap(vector<const GLchar*> faces)
{
	GLuint textureID;
	glGenTextures(1, &textureID);
	glActiveTexture(GL_TEXTURE0);

	int width,height;
	unsigned char* image;

	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
		for(GLuint i = 0; i < faces.size(); i++)
		{
			image = SOIL_load_image(faces[i], &width, &height, 0, SOIL_LOAD_RGB);
			glTexImage2D(
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0,
				GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image
			);
		}

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	return textureID;
}


void getFramerate(double* lastTime, int* nbFrames)
{
	double currentTime = glfwGetTime();
	(*nbFrames)++;
	if ( currentTime - *lastTime >= 1.0 )
	{
		//cout << 1000.0f/double(*nbFrames) << "milliseconds/frame \n";
		//cout << "Number of frames: " << *nbFrames << "\n";
		*nbFrames = 0;
		*lastTime += 1.0;
	}
}




int main(int argc, char** argv) 
{
	if ( !glfwInit() )
	{
		fprintf( stderr, "Failed to initialize GLFW\n" );
		return -1;
	}

	// Why are these lines screwing around ?!?
	//glfwWindowHint(GLFW_SAMPLES, 4);
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	
	// Open a window and create its OpenGL context
	window = glfwCreateWindow( 1024, 768, "SPH Sloshing Simulator", NULL, NULL);
	if( window == NULL ){
		fprintf( stderr, "Failed to open GLFW window.\n" );
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	glewExperimental = true; // Initialize GLEW
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		return -1;
	}

	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE); // Ensure we can capture the escape key being pressed below
	glClearColor(0.0f, 0.0f, 0.2f, 0.0f); // Dark blue background




	// Initialize FreeType
	if(FT_Init_FreeType(&library)) {
	  fprintf(stderr, "Could not init FreeType library\n");
	  return 1;
	}

	if(FT_New_Face(library, "fonts/FreeSans.ttf", 0, &face)) {
	  fprintf(stderr, "Could not open font\n");
	  return 1;
	}

	FT_Set_Pixel_Sizes(face, 0, 24);

	GLuint freetype_shaders = LoadShaders("shaders/freetype.vs", "shaders/freetype.fs");




	// Initialize Skybox
	vector<const GLchar*> faces;
	faces.push_back("skybox/right.jpg");
	faces.push_back("skybox/left.jpg");
	faces.push_back("skybox/top.jpg");
	faces.push_back("skybox/bottom.jpg");
	faces.push_back("skybox/back.jpg");
	faces.push_back("skybox/front.jpg");

	//printf("Size: %ld \n", faces.size());

	GLuint skyboxTexture = loadCubemap(faces);

	GLuint skybox_shaders = LoadShaders("shaders/skybox.vs", "shaders/skybox.fs");

	GLuint skyboxVAO, skyboxVBO;
	glGenVertexArrays(1,&skyboxVAO);

	glBindVertexArray(skyboxVAO);
		glGenBuffers(1, &skyboxVBO);
		glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(skybox_data), skybox_data, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,0);
	glBindVertexArray(0);



	// Initialize Triangle
	GLuint triangle_shaders = LoadShaders("shaders/triangle.vs", "shaders/triangle.fs");

	GLuint triangleVAO, triangleVBO, triangleColorVBO;
	glGenVertexArrays(1,&triangleVAO);

	glBindVertexArray(triangleVAO);
		glGenBuffers(1, &triangleVBO);
		glBindBuffer(GL_ARRAY_BUFFER, triangleVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(triangle_data), triangle_data, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,0);
		glGenBuffers(1, &triangleColorVBO);
		glBindBuffer(GL_ARRAY_BUFFER, triangleColorVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(triangle_color), triangle_color, GL_STATIC_DRAW);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,0,0);
	glBindVertexArray(0);







	// Initialize simulation
	init_sph();
	create_nbr_list(&nbr_list);

	vertexdata = new vec3[sph_instance.n_particles];
	get_pos(vertexdata, &sph_instance);





	// Initialize Water
	GLuint sprite_shaders = LoadShaders("shaders/water_sprites.vs", "shaders/water_sprites.fs");
	//GLuint water_shaders = LoadShaders("shaders/water_refrac.vs", "shaders/water_refrac.fs");

	GLuint waterVAO, water_vertexVBO;
	glGenVertexArrays(1, &waterVAO);

	glBindVertexArray(waterVAO);
		glGenBuffers(1,&water_vertexVBO);
		glBindBuffer(GL_ARRAY_BUFFER, water_vertexVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * sph_instance.n_particles, &vertexdata[0][0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,0);
	glBindVertexArray(0);


	// Initialize Camera
	ProjectionMatrix = perspective(45.0f, (GLfloat)4.0 / (GLfloat)3.0, 0.1f, 10.0f);
	Position = vec3(0.0f, 0.0f, 0.0f);
	azimuth = 0.0f;
	zenith = PI/2.0f;

	// Initialize Matrices
	ModelMatrix = mat4(1.0f);
	ViewMatrix = lookAt( vec3(1.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f) );
	ProjectionMatrix = perspective(45.0f, (GLfloat)4.0 / (GLfloat)3.0, 0.1f, 100.0f);

	// Get uniform locations
	GLuint ViewMatrix_ID = glGetUniformLocation(skybox_shaders, "ViewMatrix");
	GLuint ProjectionMatrix_ID = glGetUniformLocation(skybox_shaders, "ProjectionMatrix");
	GLuint MVP_ID = glGetUniformLocation(triangle_shaders, "MVP_matrix");


	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	//glEnable(GL_CULL_FACE);
	glPointSize(4.0f);


	// Variables for FPS-counter
	double lastTime = 0.0f;
	double currentTime;
	int nbFrames = 0;
	char fps[10];


	do {
			compute_matrices_from_inputs();

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// Skybox
			glDepthMask(GL_FALSE);

			glUseProgram(skybox_shaders);
			ViewMatrix = lookAt(vec3(0.0f), DiVector, UpVector);
			glUniformMatrix4fv(ViewMatrix_ID, 1, GL_FALSE, &ViewMatrix[0][0]);
			glUniformMatrix4fv(ProjectionMatrix_ID, 1, GL_FALSE, &ProjectionMatrix[0][0]);

			glBindVertexArray(skyboxVAO);
				glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
				glDrawArrays(GL_TRIANGLES, 0, 36);
			glBindVertexArray(0);

			glDepthMask(GL_TRUE);

			// Triangle
			glUseProgram(triangle_shaders);
			glUniformMatrix4fv(MVP_ID, 1, GL_FALSE, &MVP_matrix[0][0]);
			ModelMatrix = rotate(mat4(1.0f), GLfloat(CurrentTime),vec3(1.0f,1.0f,0.0f));
			ViewMatrix = lookAt(Position, Position + DiVector, UpVector);
			MVP_matrix = ProjectionMatrix * ViewMatrix * ModelMatrix;

			glBindVertexArray(triangleVAO);
				glDrawArrays(GL_TRIANGLES, 0, 3);
			glBindVertexArray(0);

			// Render water
			get_pos(vertexdata, &sph_instance);
			//cout << "sph_instance->pos[0].x: " << sph_instance.pos[0].x << endl;
			//cout << "vertexdata[0].x: " << vertexdata[0][0] << endl;

			//glUseProgram(water_shaders);
			glUseProgram(sprite_shaders);
			glUniformMatrix4fv(ViewMatrix_ID, 1, GL_FALSE, &ViewMatrix[0][0]);
			glUniformMatrix4fv(ProjectionMatrix_ID, 1, GL_FALSE, &ProjectionMatrix[0][0]);

			glBindVertexArray(waterVAO);
				glBindBuffer(GL_ARRAY_BUFFER, water_vertexVBO); // This was the culprit!
				glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec3) * sph_instance.n_particles, &vertexdata[0][0]);
				glDrawArrays(GL_POINTS, 0, sph_instance.n_particles);
			glBindVertexArray(0);

			// Render water by using the maching cubes algorithm
			//renderMarchingCubes(&(render_instance.implicit_vol), 0, render_instance.vol_width, 0, render_instance.vol_height, 0, render_instance.vol_depth, ISO_THRESHOLD);

			elapse();

			// Text rendering (experimental)
			float sx = 2.0 / 1024;	// Needs to be changed later.
			float sy = 2.0 / 768;
			
			
			render_text(freetype_shaders, "Bubo 2000", -1 + 24 * sx,   1 - 50 * sy,    sx, sy);


			// FPS-counter
			//getFramerate(&lastTime, &nbFrames);
			currentTime = glfwGetTime();
			nbFrames++;
			if ( currentTime - lastTime >= 1.0 )
			{
				sprintf(fps, "%3.0f ms/frame", 1000.0f/float(nbFrames));
				
				//cout << 1000.0f/double(*nbFrames) << "milliseconds/frame \n";
				//cout << "Number of frames: " << *nbFrames << "\n";
				nbFrames = 0;
				lastTime += 1.0;
			}
			render_text(freetype_shaders, fps, -1 + 24 * sx, 1 - 100 * sy, sx, sy);
			
			glfwSwapBuffers(window);
			glfwPollEvents();
		}
	while ( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS && glfwWindowShouldClose(window) == 0 );
	
	// Clean up
	/*
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteBuffers(1, &colorbuffer);
	glDeleteProgram(programID);
	glDeleteVertexArrays(1, &VertexArrayID);
	*/

	glfwTerminate();
	
	return 0;
}

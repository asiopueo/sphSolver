
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



// FreeType
#include <ft2build.h>
#include FT_FREETYPE_H

#include <vector>

// Project-specific includes
#include "memory.h"
#include "common.h"
#include "physics.h"
#include "shaderLoader.h"
#include "textureLoader.h"
#include "density.h"
#include "marchingcubes.h"
#include "renderer.h"


// Model data
#include "modeldata.h"


// Definition of constants
#define CUBE_LEN_X 10
#define CUBE_LEN_Y 10
#define CUBE_LEN_Z 10
#define N_PARTICLES (CUBE_LEN_X*CUBE_LEN_Y*CUBE_LEN_Z)
#define SCAL_LEN 0.1

#define ISO_THRESHOLD 0.15
#define DENSITY_RES 0.1

const float PI = 3.1415926535;

#define RANDF() 0.2 * ((rand()/(GLfloat)RAND_MAX - 0.5))



/***********************
 * Global variables (better put them into "common.h" later)
 ***********************/

GLFWwindow* window;

// 'Dreibein' and position of the Camera
vec3 DiVector;
vec3 RiVector;
vec3 UpVector;
vec3 Position;
float azimuth = 0.0f;	// Note that glm interpretes the angles as rad.
float zenith = PI/2.0F;


// SPH simulation instance
sph_struc sph_instance;
grid_struc grid_instance;
neighbor_struc nbr_list;
density_grid dense;

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

	double currentTime = glfwGetTime();

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


void init_FreeType(FT_Library &library, FT_Face &face)
{
	if(FT_Init_FreeType(&library)) {
	  fprintf(stderr, "Could not init FreeType library\n");
	  exit(1);
	}

	if(FT_New_Face(library, "fonts/FreeSans.ttf", 0, &face)) {
	  fprintf(stderr, "Could not open font\n");
	  exit(1);
	}

	FT_Set_Pixel_Sizes(face, 0, 24);	
}




/*void init_test_cube(std::vector<vec3> &vertexdata, std::vector<vec3> &normaldata)
{
	// Create density grid:
	int edge = 20;
	int border = 2;	
	float density[(edge+2)*(edge+2)*(edge+2)];

	for (int i=0; i<(edge+2)*(edge+2)*(edge+2); i++)
		density[i] = 0.0;

	// Initialize values:
	for (int i=2+border; i<edge-border; i++)
		for (int j=2+border; j<edge-border; j++)
			for (int k=2+border; k<edge-border; k++)
				density[i+j*(edge+2)+k*(edge+2)*(edge+2)] = 10.0;
	
	for (int i=1; i<edge+1; i++) {
		for (int j=1; j<edge+1; j++) {
			for (int k=1; k<edge+1; k++) {
				gridcell cell;
				get_cellvertices(cell, density, 0.1, edge+2, edge+2, i, j, k); // stride serves as a scaling factor
				std::swap(cell.v[2], cell.v[3]);
				std::swap(cell.v[6], cell.v[7]);
				
			}
		}
	}
}*/




// Initialize the states of particles
void init_sph(int length, int width, int height, int n_particles)
{
	vec3 pos[n_particles];
	vec3 vel[n_particles];

	mat4 rot;
	mat4 rotview;

	for (int x = 0; x < length; x++)
		for (int y = 0; y < width; y++)
			for (int z = 0; z < height; z++)
			{
				int i = x + y*length + z*length*width;
				pos[i] = vec3(SCAL_LEN*(x + RANDF() - length/2), SCAL_LEN*(y + RANDF() - width/2), SCAL_LEN*(z + RANDF() - height/2));
				vel[i] = vec3(0.0f, 0.0f, 0.0f);
			}

	create_sph_instance(&sph_instance, n_particles, pos, vel);
}





// Physical calculations are done here!
void elapse(void)
{
	elapse_water(&sph_instance, &grid_instance, &nbr_list);
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
	mat4 ModelMatrix;
	mat4 ViewMatrix;
	mat4 ProjectionMatrix;
	mat4 VP_matrix, MVP_matrix;

	
	// FreeType
	FT_Library library;
	FT_Face face;


	if ( !glfwInit() )
	{
		fprintf( stderr, "Failed to initialize GLFW\n" );
		return -1;
	}

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
	init_FreeType(library, face);
	GLuint freetype_shaders = LoadShaders("shaders/freetype.vs", "shaders/freetype.fs");




	// Initialize Skybox
	vector<const GLchar*> faces;
	faces.push_back("skybox/right.jpg");
	faces.push_back("skybox/left.jpg");
	faces.push_back("skybox/top.jpg");
	faces.push_back("skybox/bottom.jpg");
	faces.push_back("skybox/back.jpg");
	faces.push_back("skybox/front.jpg");

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


	std::vector<vec3> vertexdata;
	std::vector<vec3> normaldata;


	//init_test_cube(vertexdata, normaldata);

	// Initialize simulation
	init_sph(CUBE_LEN_X, CUBE_LEN_Y, CUBE_LEN_Z, N_PARTICLES);
	create_nbr_list(&nbr_list);
	// Density stamp
	density_stamp stamp;
	alloc_density_stamp(&stamp, 5, 5, 5, DENSITY_RES, 0.2);
	// Density grid allocation:
	alloc_density_grid(&dense, &sph_instance, DENSITY_RES);
	// Missing here: desity cells need volume calculation

	assign_density_to_grid(&dense, &stamp, &sph_instance);
	polygonize_density(dense, vertexdata, normaldata, ISO_THRESHOLD);




	// Initialize Water
	GLuint sprite_shaders = LoadShaders("shaders/water_sprites.vs", "shaders/water_sprites.fs");
	GLuint water_shaders = LoadShaders("shaders/water_refrac.vs", "shaders/water_refrac.fs");

	GLuint waterVAO, water_vertexVBO, water_normalVBO;
	glGenVertexArrays(1, &waterVAO);

	glBindVertexArray(waterVAO);
		glGenBuffers(1, &water_vertexVBO);
		glBindBuffer(GL_ARRAY_BUFFER, water_vertexVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * vertexdata.size(), &vertexdata[0][0], GL_DYNAMIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*) 0);

		glGenBuffers(1, &water_normalVBO);
		glBindBuffer(GL_ARRAY_BUFFER, water_normalVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * normaldata.size(), &normaldata[0][0], GL_DYNAMIC_DRAW);
		
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*) 0);
	glBindVertexArray(0);





	vec3 spritedata[sph_instance.n_particles];
	
	GLuint spriteVAO, sprite_vertexVBO;
	glGenVertexArrays(1, &spriteVAO);
	glBindVertexArray(spriteVAO);
		glGenBuffers(1, &sprite_vertexVBO);
		glBindBuffer(GL_ARRAY_BUFFER, sprite_vertexVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * sph_instance.n_particles, &spritedata[0][0], GL_STATIC_DRAW);
		//glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * vertexdata.size(), &vertexdata[0][0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glBindVertexArray(0);



	// Initialize Camera
	ProjectionMatrix = perspective(45.0f, (GLfloat)4.0 / (GLfloat)3.0, 0.1f, 10.0f);
	Position = vec3(-3.0f, 0.0f, 0.0f);
	azimuth = PI/2.;
	zenith = PI/2.0f;

	// Initialize Matrices
	ModelMatrix = mat4(1.0f);
	ViewMatrix = lookAt( vec3(1.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f) );
	ProjectionMatrix = perspective(45.0f, (GLfloat)4.0 / (GLfloat)3.0, 0.1f, 100.0f);



	// Get uniform locations
	GLuint ViewMatrix_ID = glGetUniformLocation(skybox_shaders, "ViewMatrix");
	GLuint ProjectionMatrix_ID = glGetUniformLocation(skybox_shaders, "ProjectionMatrix");


	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	//glEnable(GL_CULL_FACE); // Need to fix the 'cull' for the fonts
	glPointSize(4.0f);


	// Variables for FPS-counter
	double lastTime = 0.0f;
	double currentTime;
	int nbFrames = 0;
	char fps[30];


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

			ModelMatrix = rotate(mat4(1.0f), GLfloat(currentTime),vec3(1.0f,1.0f,0.0f));
			ViewMatrix = lookAt(Position, Position + DiVector, UpVector);
			MVP_matrix = ProjectionMatrix * ViewMatrix * ModelMatrix;

			// Triangle
			//render_triangle(triangle_shaders, MVP_matrix, triangleVAO);

			// Render water
			glUseProgram(water_shaders);
			ModelMatrix = mat4(1.0f);
			glUniformMatrix4fv(glGetUniformLocation(water_shaders, "ModelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
			glUniformMatrix4fv(glGetUniformLocation(water_shaders, "ViewMatrix"), 1, GL_FALSE, &ViewMatrix[0][0]);
			glUniformMatrix4fv(glGetUniformLocation(water_shaders, "ProjectionMatrix"), 1, GL_FALSE, &ProjectionMatrix[0][0]);
			glUniform3fv(glGetUniformLocation(water_shaders, "cameraPos"), 1, &Position[0]);





			
			elapse();
			alloc_density_grid(&dense, &sph_instance, DENSITY_RES);
			assign_density_to_grid(&dense, &stamp, &sph_instance);
			
			vertexdata.clear();
			normaldata.clear();
			polygonize_density(dense, vertexdata, normaldata, ISO_THRESHOLD);

			//cout << vertexdata.size() << endl;

			GLuint positionLocation = glGetAttribLocation(water_shaders, "position");
			GLuint normalLocation = glGetAttribLocation(water_shaders, "normal");

			glBindVertexArray(waterVAO);
				glBindBuffer(GL_ARRAY_BUFFER, water_vertexVBO);
				glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * vertexdata.size(), &vertexdata[0][0], GL_DYNAMIC_DRAW);
				//glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec3) * vertexdata.size(), &vertexdata[0][0]);
				glBindBuffer(GL_ARRAY_BUFFER, water_normalVBO);
				glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * vertexdata.size(), &normaldata[0][0], GL_DYNAMIC_DRAW);
				//glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec3) * normaldata.size(), &normaldata[0][0]);

				glDrawArrays(GL_TRIANGLES, 0, vertexdata.size());
			glBindVertexArray(0);



			// Sprites
			get_pos(spritedata, &sph_instance);

			glUseProgram(sprite_shaders);
			glUniformMatrix4fv(glGetUniformLocation(sprite_shaders, "ViewMatrix"), 1, GL_FALSE, &ViewMatrix[0][0]);
			glUniformMatrix4fv(glGetUniformLocation(sprite_shaders, "ProjectionMatrix"), 1, GL_FALSE, &ProjectionMatrix[0][0]);
			glUniform3fv(glGetUniformLocation(sprite_shaders, "cameraPos"), 1, &Position[0]);

			glBindVertexArray(spriteVAO);
				glBindBuffer(GL_ARRAY_BUFFER, sprite_vertexVBO);
				glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec3) * sph_instance.n_particles, &spritedata[0][0]);
				//glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec3) * vertexdata.size(), &vertexdata[0][0]);
				glDrawArrays(GL_POINTS, 0, sph_instance.n_particles);
				//glDrawArrays(GL_POINTS, 0, sizeof(glm::vec3) * vertexdata.size());
			glBindVertexArray(0);
			



			// Text rendering (experimental)
			float sx = 2.0 / 1024;	// Needs to be changed later.
			float sy = 2.0 / 768;
			
			render_text(freetype_shaders, "Bubo 2000", -1 + 24 * sx,   1 - 50 * sy,    sx, sy, library, face);


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
			render_text(freetype_shaders, fps, -1 + 24 * sx, 1 - 100 * sy, sx, sy, library, face);
			
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

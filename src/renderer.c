

#include <GL/glew.h>

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
#include "renderer.h"


// Constructor
Object::Object()
{
	glGenVertexArrays(1,&VAO);
	glGenBuffers(1, &vertexVBO);
	glGenBuffers(1, &normalVBO);


	glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, vertexVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(mesh->vertices), &mesh->vertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0,3, GL_FLOAT, GL_FALSE, 0, 0);
		glBindBuffer(GL_ARRAY_BUFFER, normalVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(mesh->normals), &mesh->normals, GL_STATIC_DRAW);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,0,0);
	glBindVertexArray(0);
}








void render_triangle(GLuint shaders, glm::mat4 ModelMatrix, glm::mat4 ViewMatrix, glm::mat4 ProjectionMatrix, GLuint triangleVAO)
{
	glUseProgram(shaders);
	glUniformMatrix4fv(glGetUniformLocation(shaders, "ModelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shaders, "ViewMatrix"), 1, GL_FALSE, &ViewMatrix[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shaders, "ProjectionMatrix"), 1, GL_FALSE, &ProjectionMatrix[0][0]);
	glBindVertexArray(triangleVAO);
		glDrawArrays(GL_TRIANGLES, 0, 3);
	glBindVertexArray(0);	
}


void render_water(GLuint shaders, glm::mat4 ModelMatrix, glm::mat4 ViewMatrix, glm::mat4 ProjectionMatrix, glm::vec3 Position, GLuint vertexVbo, GLuint normalVbo, GLuint vao, int size)
{
	glUseProgram(shaders);
	glUniformMatrix4fv(glGetUniformLocation(shaders, "ModelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shaders, "ViewMatrix"), 1, GL_FALSE, &ViewMatrix[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shaders, "ProjectionMatrix"), 1, GL_FALSE, &ProjectionMatrix[0][0]);
	glUniform3fv(glGetUniformLocation(shaders, "cameraPos"), 1, &Position[0]);

	//GLuint positionLocation = glGetAttribLocation(water_shaders, "position");
	//GLuint normalLocation = glGetAttribLocation(water_shaders, "normal");

	glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vertexVbo);
		glBindBuffer(GL_ARRAY_BUFFER, normalVbo);
		glDrawArrays(GL_TRIANGLES, 0, size);
	glBindVertexArray(0);
}


void render_text(GLint freetype_shdrs, const char *text, float x, float y, float sx, float sy, FT_Library library, FT_Face face)
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

void render_skybox(GLuint shaders, glm::mat4 ModelMatrix, glm::mat4 ViewMatrix, glm::mat4 ProjectionMatrix, GLuint skyboxVAO, GLuint skyboxTexture)
{
	glDepthMask(GL_FALSE);
	glUseProgram(shaders);

	GLuint ViewMatrix_ID = glGetUniformLocation(shaders, "ViewMatrix");
	GLuint ProjectionMatrix_ID = glGetUniformLocation(shaders, "ProjectionMatrix");

	glUniformMatrix4fv(ViewMatrix_ID, 1, GL_FALSE, &ViewMatrix[0][0]);
	glUniformMatrix4fv(ProjectionMatrix_ID, 1, GL_FALSE, &ProjectionMatrix[0][0]);

	glBindVertexArray(skyboxVAO);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
		glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
	glDepthMask(GL_TRUE);
}

void render_particles(GLuint shaders, glm::mat4 ModelMatrix, glm::mat4 ViewMatrix, glm::mat4 ProjectionMatrix, glm::vec3 Position, GLuint vertexVBO, GLuint vao, int n_particles)
{
	glUseProgram(shaders);
	glUniformMatrix4fv(glGetUniformLocation(shaders, "ViewMatrix"), 1, GL_FALSE, &ViewMatrix[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shaders, "ProjectionMatrix"), 1, GL_FALSE, &ProjectionMatrix[0][0]);
	glUniform3fv(glGetUniformLocation(shaders, "cameraPos"), 1, &Position[0]);

	glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vertexVBO);
		glDrawArrays(GL_POINTS, 0, n_particles);
		//glDrawArrays(GL_POINTS, 0, sizeof(glm::vec3) * vertexdata.size());
	glBindVertexArray(0);
}

void render(Object obj)
{
	
}
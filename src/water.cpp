#include "../include/bunny.hpp"
#include "../include/loader.hpp"
#include <iostream>

Bunny::Bunny(std::vector<glm::vec3> vertices, std::vector<glm::vec3> normals)
{
	bunny_shaders = LoadShaders("shaders/bunny.vs", "shaders/bunny.fs");

	glGenVertexArrays(1, &bunnyVAO);

	glBindVertexArray(bunnyVAO);
		glGenBuffers(1, &bunnyVBO);
		glBindBuffer(GL_ARRAY_BUFFER, bunnyVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*vertices.size(), &vertices[0], GL_STATIC_DRAW);
		
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		
		glGenBuffers(1, &bunnyNormalVBO);
		glBindBuffer(GL_ARRAY_BUFFER, bunnyNormalVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*normals.size(), &normals[0], GL_STATIC_DRAW);
		
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glBindVertexArray(0);
}


Bunny::~Bunny(void)
{
	glDeleteBuffers(1, &bunnyVBO);
	glDeleteBuffers(1, &bunnyNormalVBO);
	glDeleteProgram(bunny_shaders);
	glDeleteVertexArrays(1, &bunnyVAO);
}


void Bunny::draw(glm::mat4& ModelMatrix, glm::mat4& ViewMatrix, glm::mat4& ProjectionMatrix, glm::vec3& Position)
{
	glUseProgram(bunny_shaders);
	glUniformMatrix4fv(glGetUniformLocation(bunny_shaders, "ModelMatrix"), 		1, GL_FALSE, &ModelMatrix[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(bunny_shaders, "ViewMatrix"), 		1, GL_FALSE, &ViewMatrix[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(bunny_shaders, "ProjectionMatrix"), 	1, GL_FALSE, &ProjectionMatrix[0][0]);
	glUniform3fv(glGetUniformLocation(bunny_shaders, "cameraPos"), 				1, &Position[0]);
	
	glBindVertexArray(bunnyVAO);
		//glBindTexture(GL_TEXTURE_bunny_MAP, skyboxTexture);
		glDrawArrays(GL_TRIANGLES, 0, 20600);
	glBindVertexArray(0);
}
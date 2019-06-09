#include "../include/bunny.hpp"
#include "../include/loader.hpp"
#include <iostream>

Water::Water(std::vector<glm::vec3> vertices, std::vector<glm::vec3> normals)
{
	water_shaders = LoadShaders("shaders/water_shaders.vs", "shaders/water_shaders.fs");

	glGenVertexArrays(1, &waterVAO);

	glBindVertexArray(waterVAO);
		glGenBuffers(1, &waterVBO);
		glBindBuffer(GL_ARRAY_BUFFER, water);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*vertices.size(), &vertices[0], GL_STATIC_DRAW);
		
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		
		glGenBuffers(1, &waterNormalVBO);
		glBindBuffer(GL_ARRAY_BUFFER, waterNormalVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*normals.size(), &normals[0], GL_STATIC_DRAW);
		
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glBindVertexArray(0);
}


Water::~Water(void)
{
	glDeleteBuffers(1, &waterVBO);
	glDeleteBuffers(1, &waterNormalVBO);
	glDeleteProgram(water_shaders);
	glDeleteVertexArrays(1, &waterVAO);
}


void Water::draw(glm::mat4& ModelMatrix, glm::mat4& ViewMatrix, glm::mat4& ProjectionMatrix, glm::vec3& Position)
{
	glUseProgram(water_shaders);
	glUniformMatrix4fv(glGetUniformLocation(water_shaders, "ModelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(water_shaders, "ViewMatrix"), 1, GL_FALSE, &ViewMatrix[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(water_shaders, "ProjectionMatrix"), 1, GL_FALSE, &ProjectionMatrix[0][0]);
	glUniform3fv(glGetUniformLocation(water_shaders, "cameraPos"), 1, &Position[0]);
	
	glBindVertexArray(waterVAO);
		//glBindTexture(GL_TEXTURE_bunny_MAP, skyboxTexture);
		glDrawArrays(GL_TRIANGLES, 0, 20600);
	glBindVertexArray(0);
}
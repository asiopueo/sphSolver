
#define GLM_FORCE_RADIANS   // Care about this later.
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <GLFW/glfw3.h>

const float PI = 3.1415926535;

#include <cmath>
#include "camera.h"
#include "common.h"



Camera::Camera() 
{
	Position = glm::vec3(-3.0f, 0.0f, 0.0f);
	azimuth = PI/2.;
	zenith = PI/2.0f;
}

Camera::Camera(glm::vec3 position, glm::vec3 direction, glm::vec3 up) 
{
	Position = position;
	DiVector = direction;
	UpVector = up;
}


void Camera::computeMatricesFromInputs(GLFWwindow* window)
{
	double xPos, yPos;
	glm::vec3 MovementDirection;

	double currentTime = glfwGetTime();

	glfwGetCursorPos(window, &xPos, &yPos);


	azimuth -= 0.0001f*float(xPos-512);
	zenith += 0.0001f*float(yPos-384);

	zenith = clamp(0.0f, zenith, PI);

	// Für die Konventionen des Koordinatensystems fehlt mir noch das nötige Verständnis.
	DiVector = glm::vec3(sin(azimuth) * sin(zenith), 
								   cos(zenith), 
					cos(azimuth) * sin(zenith));
	RiVector = glm::vec3(sin(azimuth - PI/2.0f), 
					0.0f, 
					cos(azimuth - PI/2.0f));
	UpVector = cross(RiVector, DiVector);

	// WASD-movements
	if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		Position += glm::mat3(0.01f)*DiVector;

	if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		Position -= glm::mat3(0.01f)*DiVector;

	if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		Position += glm::mat3(0.01f)*RiVector;

	if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		Position -= glm::mat3(0.01f)*RiVector;

	// Altitude control
	if(glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
		Position += glm::mat3(0.01f)*glm::vec3(0.0f, 1.0f, 0.0f);

	if(glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
		Position -= glm::mat3(0.01f)*glm::vec3(0.0f, 1.0f, 0.0f);
}



glm::mat4 Camera::getViewMatrix()
{
	return glm::lookAt(Position, Position + DiVector, UpVector); 
}
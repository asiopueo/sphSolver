class Camera {
private:
	glm::vec3 DiVector;
	glm::vec3 RiVector;
	glm::vec3 UpVector;
	glm::vec3 Position;

	float azimuth = 0.0f;	// Note that glm interpretes the angles as rad.
	float zenith = PI/2.0F;

public:
	Camera();
	Camera(glm::vec3 position, glm::vec3 pos, glm::vec3 up);
	void computeMatricesFromInputs(GLFWwindow* window);
	glm::mat4 getViewMatrix();


};

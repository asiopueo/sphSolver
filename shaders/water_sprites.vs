#version 330 core
layout (location = 0) in vec3 position;

//out vec3 fragmentColor;

uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;

void main()
{
		gl_Position = ProjectionMatrix * ViewMatrix * vec4(position, 1.0);
		//fragmentColor = vec3(0.0f,gl_Position.z,0.0f);
}

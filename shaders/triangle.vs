#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 vertexColor;

out vec3 fragmentColor;

uniform mat4 ModelMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;

void main()
{
	mat4 MVP_matrix = ProjectionMatrix * ViewMatrix * ModelMatrix;
    gl_Position = MVP_matrix * vec4(position, 1.0);
    fragmentColor = vertexColor;
} 

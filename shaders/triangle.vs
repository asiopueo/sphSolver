#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 vertexColor;

out vec3 fragmentColor;

uniform mat4 MVP_matrix;


void main()
{
    gl_Position = MVP_matrix * vec4(position, 1.0);
    fragmentColor = vertexColor;
} 

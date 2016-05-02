#version 330 core

layout (location = 0) in vec3 position;
out vec3 texture_coords;

uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;


void main()
{
	gl_Position = ProjectionMatrix * ViewMatrix * vec4(position, 1.0);
    texture_coords = position;
}

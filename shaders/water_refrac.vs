#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

out vec3 Normal;
out vec3 Position;

uniform mat4 ModelMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;

void main()
{
		gl_Position = ProjectionMatrix * ViewMatrix * ModelMatrix * vec4(position, 1.0);
		Normal = vec3(transpose(inverse(ModelMatrix)) * vec4(normal, 1.0));
		Position = vec3(ModelMatrix * vec4(position, 1.0f));
}

#version 330 core

in vec3 Normal;
in vec3 Position;

out vec4 color;

uniform vec3 cameraPos;
uniform samplerCube skybox;

void main()
{
	/*
	vec3 I = vec3(normalize(Position - cameraPos));
	vec3 R = vec3(reflect(I, normalize(Normal)));
	color = texture(skybox, R);
	*/
	float ratio = 1.00 / 1.52;
    vec3 I = normalize(Position - cameraPos);
    vec3 R = refract(I, normalize(Normal), ratio);
    color = texture(skybox, R);
}



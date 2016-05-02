#version 330

in vec2 texcoord;
out vec4 color;

uniform sampler2D text;

void main(void)
{
	color = vec4(1.0f, 1.0f, 1.0f, texture(text, texcoord).r);
}

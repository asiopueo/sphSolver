#version 330

layout (location = 0) in vec4 coord;
out vec2 texcoord;

void main(void)
{
	gl_Position = vec4(coord.xy, 0.0f, 1.0f);
	texcoord = coord.zw;
}

#version 400 core

in vec2 UV;

out vec4 color;

uniform sampler2D healthTex;
uniform sampler2D healthEmptyTex;

void main(void)
{
	color = vec4(texture(healthTex, UV));
}
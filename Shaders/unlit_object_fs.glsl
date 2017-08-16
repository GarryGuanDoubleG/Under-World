#version 400 core
out vec4 color;

uniform vec4 colorMod;

void main()
{
	color = vec4(colorMod, 1.0f);
}
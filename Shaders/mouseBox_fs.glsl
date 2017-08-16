#version 400 core
out vec4 color;

uniform vec3 boxColor;

void main()
{ 
	color = vec4(boxColor.rgb, .4f);
}
#version 450 core
layout (location = 0) in vec3 position;

uniform mat4 model;
uniform mat4 lightSpaceMat;

void main()
{
	gl_Position = lightSpaceMat * model * vec4(position, 1.0f);
}
#version 400 core
layout (location = 0) in vec2 position;
layout (location = 1) in vec2 uv;

out vec2 UV;
out vec3 viewPos;

uniform vec3 cameraPos;

void main()
{
	viewPos = cameraPos;
	gl_Position = vec4(position, 0, 1.0f);

	UV = uv;
} 
#version 450 core
layout (location = 0) in vec2 position;
layout (location = 1) in vec2 uv;

#pragma include "common.inc.glsl"

out vec2 UV;
out vec3 viewPos;

uniform vec3 cameraPos;

void main()
{
	viewPos = cameraPos;
	UV = uv;

	gl_Position = vec4(position, 0, 1.0f);
}

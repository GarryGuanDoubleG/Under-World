#version 450 core
layout (location = 0) in vec2 position;
layout (location = 1) in vec2 uv;

layout (binding = 0) uniform sampler2D gPosition;

layout (binding = 1) uniform sampler3D highNoise;
layout (binding = 2) uniform sampler3D lowNoise;
layout (binding = 3) uniform sampler2D curlNoise;

layout(binding = 4) uniform sampler2D weatherMap;

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

#version 400 core
layout (location = 0) in vec2 position;
layout (location = 1) in vec2 uv;

out vec2 UV;

uniform vec2 scale;
uniform vec2 screen_pos;

void main()
{
	vec2 scaledPos = position;
	scaledPos = (scaledPos + vec2(1.0, -1.0)) * scale;
	scaledPos = scaledPos - vec2(1.0 , -1.0);
	scaledPos += screen_pos;

    gl_Position = vec4(scaledPos.xy, 0.0f, 1.0f);

	UV = uv;
} 
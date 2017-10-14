#version 450 core
layout (location = 0) in vec3 verts;
layout (location = 1) in vec2 uv;

out vec2 UV;

void main()
{
    UV = uv;
    gl_Position = vec4(verts, 1.0);
}
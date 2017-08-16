#version 400 core

layout (location = 0) in vec3 verts;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 verts_UV;

//uniforms
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main(void)
{
	gl_Position = projection * view * model * vec4(verts,1.0f);
}

#version 400

layout (location = 0) in vec3 verts;

//uniforms
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main(void)
{
	gl_Position = projection * view * model * vec4(verts,1.0f);
}

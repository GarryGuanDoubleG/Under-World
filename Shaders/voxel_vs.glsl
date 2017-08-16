#version 400 core
layout (location = 0) in vec3 verts;
layout (location = 1) in vec3 normal;

out vec3 vs_normal;
out vec3 FragPos;

//out vec2 UV;
//uniforms
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main(void)
{
	gl_Position = projection * view * vec4(verts,1.0f);

	vs_normal = mat3(transpose(inverse(model))) * normal;	
	FragPos = vec3(model * vec4(verts,1.0f));
}
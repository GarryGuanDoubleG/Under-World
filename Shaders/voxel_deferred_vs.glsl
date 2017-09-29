#version 430 core
layout (location = 0) in vec3 verts;
layout (location = 1) in vec3 normal;
layout (location = 2) in int textureID;

out VS_OUT
{
	vec3 FragPos;
	vec3 normal;
	flat int texID;
}vs_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	vs_out.normal = transpose(inverse(mat3(model))) * normal;	
	vs_out.FragPos = vec3(model * vec4(verts,1.0f));
	vs_out.texID = textureID;

	gl_Position = projection * view * model * vec4(verts,1.0f);
}
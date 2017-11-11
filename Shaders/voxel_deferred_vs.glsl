#version 450 core
layout (location = 0) in vec3 verts;
layout (location = 1) in vec3 normal;
layout (location = 2) in int textureID;


out VS_OUT
{
	vec3 ScreenFragPos;
	vec3 WorldFragPos;
	vec3 normal;
	flat int texID;
}vs_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	vec4 worldFragPos = model * vec4(verts, 1.0f);

	vs_out.normal = transpose(inverse(mat3(model))) * normal;
	vs_out.WorldFragPos = vec3(worldFragPos.xyz);
	vs_out.ScreenFragPos = (view * worldFragPos).xyz;
	vs_out.texID = textureID;

	gl_Position = projection * view * model * vec4(verts,1.0f);
}

#version 400 core

layout (location = 0) in vec3 verts;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 verts_UV;
layout (location = 3) in ivec4 boneIds;
layout (location = 4) in vec4 weights;

out vec3 vs_normal;
out vec3 FragPos;
out vec2 UV;

const int MAX_BONES = 100;

//uniforms
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform mat4 gBones[MAX_BONES];

void main(void)
{
	mat4 boneTransform = gBones[boneIds[0]] * weights[0];
	boneTransform += gBones[boneIds[1]] * weights[1];
	boneTransform += gBones[boneIds[2]] * weights[2];
	boneTransform += gBones[boneIds[3]] * weights[3];

	vec4 boneVerts = boneTransform * vec4(verts, 1.0);
	//vec4 boneVerts = boneTransform * vec4(verts, 1.0);

	gl_Position = projection * view * model * boneVerts;

	UV = verts_UV;
	vs_normal = mat3(transpose(inverse(model))) * normal;	
	FragPos = vec3(model * vec4(verts,1.0f));
}

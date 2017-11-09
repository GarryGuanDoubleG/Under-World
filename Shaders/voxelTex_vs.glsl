#version 450 core
layout (location = 0) in vec3 verts;
layout (location = 1) in vec3 normal;
layout (location = 2) in int textureID;
layout (location = 3) in int flip;

out VS_OUT
{
	vec3 normal;
	vec3 FragPos;
	vec4 FragPosLightSpace;
	flat int texID;
}vs_out;
//out vec2 UV;
//uniforms
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;


void main(void)
{
	vs_out.normal = transpose(inverse(mat3(model))) * normal;	
	vs_out.normal *= flip > 0 ? -1.0f : 1.0f;
	vs_out.FragPos = vec3(model * vec4(verts,1.0f));
	vs_out.FragPosLightSpace = lightSpaceMatrix * vec4(vs_out.FragPos, 1.0);
	vs_out.texID = textureID;

	gl_Position = projection * view * model * vec4(verts,1.0f);
}

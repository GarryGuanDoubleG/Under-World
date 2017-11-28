#version 450 core
layout (location = 0) in vec3 verts;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 verts_UV;
layout (location = 3) in vec3 tangent;

out vec2 UV;
out vec3 FragPos;
out mat3 TBN;

//uniforms
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;


void main(void)
{
	vec4 worldPos  = model * vec4(verts, 1.0f);

	FragPos = vec3(worldPos);//fragment position in view space for ssao
	UV = verts_UV;

	
	vec3 T = normalize(vec3(model * vec4(tangent, 0.0)));
	vec3 N = normalize(vec3(model * vec4(normal, 0.0)));
	vec3 B = cross(N, T);

	// re-orthogonalize T with respect to N
	T = normalize(T - dot(T, N) * N);

	TBN = mat3(T, B, N);

	gl_Position = projection * view * worldPos;
}

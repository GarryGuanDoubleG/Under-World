#version 400 core

/**Code based on Deferred Rendering of Planetary Terrains with Accurate Atmospheres by Stefan Sperlhofer */
/**http://gamedevs.org/uploads/deferred-rendering-of-planetary-terrains-with-accurate-atmospheres.pdf*/

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec2 verts_UV;

out VS_OUT
{
	vec2 UV;
}vs_out;

layout (std140) uniform uboSun
{
	vec3 g_cameraPos;
	vec3 g_sunDirection;
	float g_sunDistance;
	mat4 g_viewProj;
	float g_invFarPlane;
}

// output - rotationMatrix: matrix that lets the quadrilateral face the
// camera at all times

mat4 generateRotationMatrix()
{
	mat4 rotationMatrix;

	float3 lookAtDirection = normalize(-g_sunDirection);
	float3 baseVector0, baseVector1, baseVector2;
	// first base vector equals lookAtDirection
	baseVector0 = lookAtDirection;
	// second base vector found by crossing first base vecotr with
	// cardinal basis vector corresponding to element with least magnitude
	float3 crossVector = float3(1, 0, 0);
	float absX = abs(lookAtDirection.x);
	float absY = abs(lookAtDirection.y);
	float absZ = abs(lookAtDirection.z);

	if((absY <= absX) && (absY <= absZ))
		crossVector = float3(0.0f, 1.0f, 0.0f);
	else if((absZ <= absX) && (absZ <= absY))
		crossVector = float3(0.0f, 0.0f, 1.0f);

	baseVector1 = normalize(cross(baseVector0, crossVector));
	
	// third base vector equals crossing first and second base vector
	baseVector2 = normalize(cross(baseVector0, baseVector1));

	rotationMatrix[0] = float4(baseVector2, 0.0f);
	rotationMatrix[1] = float4(baseVector1, 0.0f);
	rotationMatrix[2] = float4(baseVector0, 0.0f);
	rotationMatrix[3] = float4(0.0f, 0.0f, 0.0f, 1.0f);

	return rotationMatrix;
}
// input - sunPosition: postion of the sun in world space
// output - translationMatrix: matrix that translates quadrilateral to world
//space position
void generateTranslationMatrix(vec3 sunPosition)
{
	mat4 translationMatrix;

	translationMatrix[0] = vec4(1.0f, 0.0f, 0.0f, 0.0f);
	translationMatrix[1] = vec4(0.0f, 1.0f, 0.0f, 0.0f);
	translationMatrix[2] = vec4(0.0f, 0.0f, 1.0f, 0.0f);
	translationMatrix[3] = vec4(sunPosition, 1.0f);

	return translationMatrix;
}

void main()
{
	vec3 sunPosition = g_cameraPos + (g_sunDirection * g_sunDistance);

	mat4 rotationMatrix, translationMatrix;
	rotationMatrix = generateRotationMatrix();
	translationMatrix = generateTranslationMatrix(sunPosition);

	mat4 worldMatrix = translationMatrix * rotationMatrix;
	mat4 mvp = g_viewProj * worldMatrix;

	vec4 pos = mvp * vec4(vertex, 1.0f);
	pos.z = pos.z * pos.w * g_invFarPlane;

	vs_out.UV = verts_UV;

	gl_Position = pos;
}

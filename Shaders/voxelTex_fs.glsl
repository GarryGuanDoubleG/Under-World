#version 400 core
out vec4 color;

in VS_OUT
{
	vec3 normal;
	vec3 FragPos;
	vec4 FragPosLightSpace;
	flat int texID;
}fs_in;

uniform vec3 viewPos;
uniform vec3 lightDirection;
uniform vec3 lightColor;
uniform vec3 voxelColor;

#define MAX_TEXTURES 15

uniform sampler2D voxelTexture[MAX_TEXTURES];
uniform sampler2D shadowMap;

vec3 getTriPlanarBlend(vec3 _wNorm){
	// in wNorm is the world-space normal of the fragment
	vec3 blending = abs( _wNorm );
	blending = normalize(max(blending, 0.00001)); // Force weights to sum to 1.0
	float b = (blending.x + blending.y + blending.z);
	blending /= vec3(b);
	return blending;
}

float when_lt(float x, float y)
{
	return max(sign(y - x), 0.0);
}

float ShadowCalculation(vec4 FragPosLightSpace)
{
	//perform perspective divide
	//light space position is in ndc [-1, 1]
	vec3 projCoords = FragPosLightSpace.xyz / FragPosLightSpace.w;
	//convert to texture coords [0, 1]
	projCoords = projCoords * 0.5 + 0.5;

	float closestDepth = texture(shadowMap, projCoords.xy).r;
	float currentDepth = projCoords.z;
	float bias = max(0.05 * (1.0 - dot(fs_in.normal, -lightDirection)), 0.05);
	float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;

	////set shadow to 0 if z coord is > 1.0
	//shadow *= when_lt(projCoords.z, 1.0);
	
	if(projCoords.z > 1.0)
		shadow = 0.0;

	return shadow;

}

void main(void)
{	
	int index = int(fs_in.texID);
	
	//calculate texture value at this fragment
	float scale = .00015f;
	vec3 blending = getTriPlanarBlend(fs_in.normal);
	vec3 xaxis = texture2D( voxelTexture[index], fs_in.FragPos.yz * scale).rgb;
	vec3 yaxis = texture2D( voxelTexture[index], fs_in.FragPos.xz * scale).rgb;
	vec3 zaxis = texture2D( voxelTexture[index], fs_in.FragPos.xy * scale).rgb;
	vec3 tex = xaxis * blending.x + yaxis * blending.y + zaxis * blending.z;

	//specular uses direction towards light source
	/*******SPECULAR ***************/

	vec3 lightDir = normalize(-lightDirection);
	vec3 norm = normalize(fs_in.normal);
	vec3 viewDir = normalize(viewPos - fs_in.FragPos);
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(viewDir, halfwayDir), 0.0), 32);
	vec3 specular =  .8f * spec * lightColor;

	//diffuse uses direction away from light source
	float diff = max(dot(norm, -lightDir), 0.0);
	vec3 diffuse = diff * lightColor * .5f;
	
	float ambientStrength = 0.2f;
	vec3 ambient = ambientStrength * lightColor;

	float shadow = ShadowCalculation(fs_in.FragPosLightSpace);
    vec3 result = (ambient + (1.0 - shadow) * (diffuse + specular)) * tex;
	//vec3 result = diffuse;
	color = vec4(result, 1.0f);
}

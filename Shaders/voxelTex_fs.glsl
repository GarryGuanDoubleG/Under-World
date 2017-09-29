 #version 400 core

 #pragma include "voxelCommon.inc.glsl"
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


uniform sampler2D shadowMap;
uniform sampler2D depthMap;

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
	vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
	
	//PCF
	for(int x = -1; x <= 1; ++x)
	{
		for(int y = -1; y <= 1; ++y)
		{
			float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
			shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
		}    
	}

	shadow /= 9.0;
	
	if(projCoords.z > 1.0)
		shadow = 0.0;

	return shadow;

}


void main(void)
{	
	int index = int(fs_in.texID);
	
	//calculate texture value at this fragment
	float scale = .0015f;
	vec3 blending = getTriPlanarBlend(fs_in.normal);
	vec3 tex = GetTriPlanarTex(fs_in.FragPos, blending, scale, index);
	vec3 norm = TriPlanarNormal(fs_in.FragPos, fs_in.normal, blending, scale);

	//specular uses direction towards light source
	/*******SPECULAR ***************/
	vec3 lightDir = normalize(-lightDirection);;
	vec3 viewDir = normalize(viewPos - fs_in.FragPos);
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(viewDir, halfwayDir), 0.0), 32);
	vec3 specular =  .8f * spec * lightColor;

	//diffuse uses direction away from light source
	float diff = max(dot(norm, -lightDir), 0.0);
	vec3 diffuse = diff * lightColor * 1.5f;
	
	float ambientStrength = 0.6f;
	vec3 ambient = ambientStrength * lightColor;

	float shadow = ShadowCalculation(fs_in.FragPosLightSpace);
    vec3 result = (ambient + (1.0 - shadow) * (diffuse + specular)) * tex;
	//vec3 result = diffuse;
	color = vec4(result, 1.0f);
}

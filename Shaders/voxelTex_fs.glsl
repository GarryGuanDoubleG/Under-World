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
#define GRASS 0
#define STONE 1
#define DIRT 2

uniform sampler2D voxelTexture[MAX_TEXTURES];
uniform sampler2D normalMap[MAX_TEXTURES];

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

vec3 getTriPlanarBlend(vec3 _wNorm){
	// in wNorm is the world-space normal of the fragment
	vec3 blending = abs( _wNorm );
	blending = normalize(max(blending, 0.0000001)); // Force weights to sum to 1.0
	float b = (blending.x + blending.y + blending.z);
	blending /= vec3(b);
	return blending;
}

vec3 GetTriPlanarTex(vec3 FragPos, vec3 blending, float scale, int index)
{
	vec3 xaxis = texture2D( voxelTexture[STONE], FragPos.yz * scale).rgb;
	vec3 yaxis = texture2D( voxelTexture[GRASS], FragPos.xz * scale).rgb;
	vec3 zaxis = texture2D( voxelTexture[STONE], FragPos.xy * scale).rgb;

	return xaxis * blending.x + yaxis * blending.y + zaxis * blending.z;
}

vec3 perturb_normal(vec3 N, vec3 B, vec3 T, vec2 texcoord, sampler2D texture)
{
	float invmax = inversesqrt(max(dot(T ,T), dot(B, B)));
	mat3 TBN =  mat3(T * invmax, B * invmax, N);

	vec3 map = texture2D(texture, texcoord).rgb;

	map = map * 2.0 - 1.0f;

	map = map * TBN;
	map = normalize(map);
	return map;
}


vec3 TriPlanarNormal(vec3 FragPos, vec3 normal, vec3 blending, float scale)
{
    vec3 duv1 = dFdx(FragPos);
    vec3 duv2 = dFdy(FragPos);

    vec3 dp1perp = cross(normal, duv1);
    vec3 dp2perp = cross(duv2, normal);

	vec3 Tx = dp2perp * duv1.x + dp1perp * duv2.x; 
    vec3 Ty = dp2perp * duv1.y + dp1perp * duv2.y; 
    vec3 Tz = dp2perp * duv1.z + dp1perp * duv2.z; 

	vec3 norm1 = perturb_normal(normal, Ty, Tz, FragPos.yz * scale, normalMap[STONE]);
	vec3 norm2 = perturb_normal(normal, Tx, Tz, FragPos.xz * scale, normalMap[GRASS]);
	vec3 norm3 = perturb_normal(normal, Tx, Ty, FragPos.xy * scale, normalMap[STONE]);

	return norm1 * blending.x + norm2 * blending.y + norm3 * blending.z;
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

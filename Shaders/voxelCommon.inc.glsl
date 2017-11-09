#define MAX_TEXTURES 15
#define GRASS 0
#define STONE 1
#define DIRT 2

uniform sampler2D voxelTexture[MAX_TEXTURES];
uniform sampler2D normalMap[MAX_TEXTURES];

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
	vec3 yaxis = texture2D( voxelTexture[STONE], FragPos.xz * scale).rgb;
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
	vec3 norm2 = perturb_normal(normal, Tx, Tz, FragPos.xz * scale, normalMap[STONE]);
	vec3 norm3 = perturb_normal(normal, Tx, Ty, FragPos.xy * scale, normalMap[STONE]);

	//x and y components should be switched for some reason
	vec3 outNorm;
	outNorm.yxz = norm1 * blending.x + norm2 * blending.y + norm3 * blending.z;
	return outNorm;

	//return norm1 * blending.x + norm2 * blending.y + norm3 * blending.z;
}
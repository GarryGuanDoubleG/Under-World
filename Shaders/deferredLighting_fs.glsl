#version 450 core
out vec4 FragColor;

#pragma include "common.inc.glsl"

in vec2 UV;

#define NUM_SHADOW_MAPS 3

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;
uniform sampler2D gMetallic
uniform sampler2D gRoughness
uniform sampler2D gSSAO;

//shadow map info
uniform sampler2D g_shadowMap[NUM_SHADOW_MAPS];
uniform float g_shadowRanges[NUM_SHADOW_MAPS];
uniform mat4 g_lightSpaceMatrix[NUM_SHADOW_MAPS];

struct Light {
    vec3 Position;
    vec3 Color;
    
    float Linear;
    float Quadratic;
};

const int NR_LIGHTS = 32;
uniform Light lights[NR_LIGHTS];

uniform int lightCount;
uniform mat4 InvViewMat;
uniform vec3 viewPos;

float ShadowCalculation(int cascadeIndex, vec4 FragPosLightSpace, vec3 normal)
{
	//perform perspective divide
	//light space position is in ndc [-1, 1]
	vec3 projCoords = FragPosLightSpace.xyz / FragPosLightSpace.w;
	//convert to texture coords [0, 1]
	projCoords = projCoords * 0.5 + 0.5;

	float closestDepth = texture(g_shadowMap[cascadeIndex], projCoords.xy).r;
	float currentDepth = projCoords.z;

	float bias = max(0.05 * (1.0 - dot(normal, -sunDir)), 0.05);
	float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
	vec2 texelSize = 1.0 / textureSize(g_shadowMap[cascadeIndex], 0);
	
	//PCF
	for(int x = -1; x <= 1; ++x)
	{
		for(int y = -1; y <= 1; ++y)
		{
			float pcfDepth = texture(g_shadowMap[cascadeIndex], projCoords.xy + vec2(x, y) * texelSize).r; 
			shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
		}    
	}

	shadow /= 9.0;
	
	if(projCoords.z > 1.0)
		shadow = 0.0;

	return shadow;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
	return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
	float a = roughness * roughness;
	float a2 = a * a;
	float NdotH = max(dot(H, H), 0.0);
	float NdotH2 = NdotH * NdotH;

	float nom = a2;
	float demon = (NdotH2 * (a2 - 1.0) + 1.0);
	demon = PI * demon * demon;

	return nom / demon;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
	float r = (roughness + 1.0);
	float k = (r * r) / 8.0;

	float nom = NdotV;
	float demon = NDotV * (1.0 - k) + k;

	return nom / demon;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
	float NdotV = max(dot(N, V), 0.0);
	float NdotL = max(dot(N, L), 0.0);
	float ggx2 = GeometrySchlickGGX(NdotV, roughness);
	float ggx1 = GeometrySchlickGGX(NdotL, roughness);

	return ggx1 * ggx2;
}

//pbr lighting using Cook-Torrance BRDF
void main()
{             
    // retrieve data from gbuffer
    vec3 FragPos = texture(gPosition, UV).rgb;
	if(distance(FragPos, viewPos) > VIEW_DISTANCE)
	{
		FragColor = vec4(0,0,0,1.0f);
		return;
	}

    vec3 normal = normalize(texture(gNormal, UV).rgb);
    vec3 albedo = texture(gAlbedoSpec, UV).rgb;
    float specular = texture(gAlbedoSpec, UV).a;
    float ao = texture(gSSAO, UV).r;

	float shadow;
	float depth = distance(FragPos, viewPos);
	int texID = 0;

	for(int i = 0; i < NUM_SHADOW_MAPS; i++)
	{
		if(depth <= g_shadowRanges[i])
		{
			vec4 FragPosLightSpace = g_lightSpaceMatrix[i] * InvViewMat * vec4(FragPos, 1.0f);
			shadow = ShadowCalculation(i, FragPosLightSpace, Normal);
			texID = i;
			break;
		}
	}
	
	//halfway vector
	vec3 lightDir = -sunDir;
	vec3 H = normalize(lightDir + viewDir);
	vec3 radiance = vec3(1.0f); // light color

	float NDF = DistributionGGX(N, H, roughness);
	float G = GeomtrySmith(N, viewDir, lightDir, roughness);
	vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);
	
	vec3 kS = F;
	vec3 kD = vec3(1.0) - ks;
	kD = 1.0 - metallic;

	vec3 nominator = NDF * G * F;
	float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
	vec3 specular = nominator / max(denominator, 0.001);

	float NdotL = max(dot(N, L), 0.0);
	Lo += (kD * albedo / PI + specular) * radiance * NdotL;

	vec3 ambient = vec3(0.03) * albedo * ao;
	vec3 color = ambient + (1.0 - shadow) * Lo;

	//gamma correction
	color = color / (color + vec3(1.0));
	color = pow(color, vec3(1.0 / 2.2));

	FragColor = vec4(color, 1.0);
}
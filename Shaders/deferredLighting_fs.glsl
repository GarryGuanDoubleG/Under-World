#version 450 core
out vec4 FragColor;

#pragma include "common.inc.glsl"

in vec2 UV;

#define NUM_SHADOW_MAPS 3

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;
uniform sampler2D ssao;

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

void main()
{             
    // retrieve data from gbuffer
    vec3 FragPos = texture(gPosition, UV).rgb;
	if(distance(FragPos, viewPos) > VIEW_DISTANCE)
	{
		FragColor = vec4(0,0,0,1.0f);
		return;
	}

    vec3 Normal = normalize(texture(gNormal, UV).rgb);
    vec3 Diffuse = texture(gAlbedoSpec, UV).rgb;
    float Specular = texture(gAlbedoSpec, UV).a;
    float AmbientOcclusion = texture(ssao, UV).r;

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
    // diffuse lighting
    vec3 lighting  = Diffuse * .3f * AmbientOcclusion; // hard-coded ambient component
	vec3 viewDir = normalize(-FragPos);
	vec3 lightDir = -sunDir;
	vec3 diffuse = max(dot(Normal, lightDir), 0.0) * Diffuse;
	
    // specular
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(Normal, halfwayDir), 0.0), 16.0);
    vec3 specular = vec3(1.0f) * spec * Specular;

    lighting += (1.0f - shadow) * (diffuse + specular); 
	FragColor = vec4(lighting, 1.0f);
}
#version 450 core
out vec4 FragColor;

#pragma include "common.inc.glsl"

in vec2 UV;

#define NUM_SHADOW_MAPS 3

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;
uniform sampler2D gAO;
uniform sampler2D gMetallic;
uniform sampler2D gRoughness;
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

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = M_PI * denom * denom;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

// ----------------------------------------------------------------------------
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

// ----------------------------------------------------------------------------
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

float CascadeShadows(vec3 FragPos, vec3 normal)
{
	float shadow;
	float depth = distance(FragPos, viewPos);
	int texID = 0;

	for(int i = 0; i < NUM_SHADOW_MAPS; i++)
	{
		if(depth <= g_shadowRanges[i])
		{
			vec4 FragPosLightSpace = g_lightSpaceMatrix[i] * vec4(FragPos, 1.0f);
			shadow = ShadowCalculation(i, FragPosLightSpace, normal);
			texID = i;
			break;
		}
	}

	return shadow;
}

vec3 CalculateLighting(vec3 viewDir, vec3 normal, vec3 albedo, float roughness, float metallic)
{
	vec3 lightDir = -normalize(sunDir);
	vec3 radiance = vec3(10.0f); // light color
	vec3 halfVec = normalize(lightDir + viewDir);

	vec3 N = normal;
	vec3 H = halfVec;
	vec3 V = viewDir;
	vec3 L = lightDir;

	vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);

    // reflectance equation
    vec3 Lo = vec3(0.0);
	// calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)    
	// Cook-Torrance BRDF
    float NDF = DistributionGGX(N, H, roughness);   
    float G   = GeometrySmith(N, V, L, roughness);      
    vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);
           
    vec3 nominator    = NDF * G * F; 
    float denominator = 4 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001; // 0.001 to prevent divide by zero.
    vec3 specular = nominator / denominator;
        
    // kS is equal to Fresnel
    vec3 kS = F;
    // for energy conservation, the diffuse and specular light can't
    // be above 1.0 (unless the surface emits light); to preserve this
    // relationship the diffuse component (kD) should equal 1.0 - kS.
    vec3 kD = vec3(1.0) - kS;
    // multiply kD by the inverse metalness such that only non-metals 
    // have diffuse lighting, or a linear blend if partly metal (pure metals
    // have no diffuse light).
    kD *= 1.0 - metallic;	  

    // scale light by NdotL
    float NdotL = max(dot(N, L), 0.0);        

    // add to outgoing radiance Lo
    Lo += (kD * albedo / M_PI + specular) * radiance * NdotL;  // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again

	return Lo;
}

vec3 PhongLighting(vec3 albedo, vec3 normal, vec3 viewDir, vec3 ao, float shadow)
{
		// diffuse lighting
    vec3 lighting  = albedo * .3f * ao; // hard-coded ambient component
	//vec3 viewDir = normalize(-FragPos);
	vec3 lightDir = -sunDir;
	vec3 diffuse = max(dot(normal, lightDir), 0.0) * albedo;
	
    // specular
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 16.0);
    vec3 specular = vec3(1.0f) * spec * .75f;

    lighting += (1.0f - shadow) * (diffuse + specular); 
	return lighting;
}

//pbr lighting using Cook-Torrance BRDF
void main()
{             
    // retrieve data from gbuffer
    vec3 FragPos = texture(gPosition, UV).rgb;
	FragPos = (InvViewMat * vec4(FragPos, 1.0f)).rgb;
	if(distance(FragPos, viewPos) > VIEW_DISTANCE)
	{
		FragColor = vec4(0,0,0,1.0f);
		return;
	}

    vec3 albedo = pow(texture(gAlbedoSpec, UV).rgb, vec3(2.2));
	vec3 normal = texture(gNormal, UV).rgb;
	float roughness = texture(gRoughness, UV).r;
	float metallic = texture(gMetallic, UV).r;
    float ao = texture(gSSAO, UV).r;// + texture(gAO, UV).r;

	vec3 viewDir = normalize(viewPos - FragPos);
	//vec3 viewDir = normalize(viewPos - vec3(InvViewMat * vec4(FragPos, 1.0f)));

	//calculate shadows
	float shadow = CascadeShadows(FragPos, normal);
	//float shadow = 0;
	//physically based rendering
	vec3 Lo = CalculateLighting(viewDir, normal, albedo, roughness, metallic);

	//ambient component
	vec3 ambient = vec3(0.03) * albedo * ao;
	//vec3 ambient = vec3(0.2) * albedo;

	vec3 color = ambient + (1.0 - shadow) * Lo;
	 ////HDR tonemapM_PIng
	color = color / (color + vec3(1.0));
	//gamma correction with shadowing
	color = pow(color, vec3(1.0 / 2.2));

	FragColor = vec4(color, 1.0);
}
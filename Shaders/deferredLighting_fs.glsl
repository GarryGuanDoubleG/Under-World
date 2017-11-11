#version 450 core
out vec4 FragColor;

#pragma include "common.inc.glsl"

in vec2 UV;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;
uniform sampler2D ssao;

struct Light {
    vec3 Position;
    vec3 Color;
    
    float Linear;
    float Quadratic;
};

const int NR_LIGHTS = 32;
uniform Light lights[NR_LIGHTS];

uniform int lightCount;
uniform vec3 viewPos;

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
	//float AmbientOcclusion = 1.f;

    // then calculate lighting as usual
    vec3 lighting  = Diffuse * 0.3 * AmbientOcclusion; // hard-coded ambient component
	vec3 viewDir = normalize(-FragPos);
	vec3 lightDir = sunDir;
	vec3 diffuse = max(dot(Normal, lightDir), 0.0) * Diffuse * .5f;
	
    // specular
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(Normal, halfwayDir), 0.0), 16.0);
    vec3 specular = vec3(1.0f) * spec * Specular;

    lighting += diffuse + specular;        

    FragColor = vec4(lighting, 1.0);
	//FragColor = vec4(Normal, 1.0);
}
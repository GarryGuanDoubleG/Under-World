#version 450 core
out vec4 FragColor;

in vec2 UV;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;

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
uniform vec3 sunDir;

void main()
{             
    // retrieve data from gbuffer
    vec3 FragPos = texture(gPosition, UV).rgb;
	if(distance(FragPos, viewPos) > 40000)
	{
		FragColor = vec4(0,0,0,1.0f);
		return;
	}
    vec3 Normal = normalize(texture(gNormal, UV).rgb);
	//Normal = Normal * 2.0 - 1.0;
    vec3 Diffuse = texture(gAlbedoSpec, UV).rgb;
    float Specular = texture(gAlbedoSpec, UV).a;
    
    // then calculate lighting as usual
    vec3 lighting  = Diffuse * 0.2; // hard-coded ambient component
    vec3 viewDir  = normalize(viewPos - FragPos);
	vec3 lightDir = sunDir;
	//vec3 lightDir = vec3(-.2f, -1.f, -0.3f);
	//lightDir *= -1;
    //for(int i = 0; i < lightCount; ++i)
    //{
    //    // diffuse
    //    vec3 lightDir = normalize(lights[i].Position - FragPos);
    //vec3 diffuse = max(dot(Normal, lightDir), 0.0) * Diffuse * .5f;
	vec3 diffuse = max(dot(Normal, lightDir), 0.0) * Diffuse * .35f;
	
    // specular
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(Normal, halfwayDir), 0.0), 16.0);
    vec3 specular = vec3(1.0f) * spec * Specular;
        // attenuation
        /*float distance = length(lights[i].Position - FragPos);
        float attenuation = 1.0 / (1.0 + lights[i].Linear * distance + lights[i].Quadratic * distance * distance);
        diffuse *= attenuation;
        specular *= attenuation*/;
    lighting += diffuse + specular;        
	//lighting = diffuse;
    //}
	
    FragColor = vec4(lighting, 1.0);
	//FragColor = vec4(Normal, 1.0);
	//FragColor = vec4(Diffuse, 1.0f);
}
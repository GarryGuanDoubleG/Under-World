#version 330 core
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
    vec3 Normal = texture(gNormal, UV).rgb;
    vec3 Diffuse = texture(gAlbedoSpec, UV).rgb;
    float Specular = texture(gAlbedoSpec, UV).a;
    
    // then calculate lighting as usual
    vec3 lighting  = Diffuse * 0.3; // hard-coded ambient component
    vec3 viewDir  = normalize(viewPos - FragPos);
	vec3 lightDir = sunDir * -1;
	//vec3 lightDir = vec3(-.2f, -1.f, -0.3f);
	//lightDir *= -1;
    //for(int i = 0; i < lightCount; ++i)
    //{
    //    // diffuse
    //    vec3 lightDir = normalize(lights[i].Position - FragPos);
    vec3 diffuse = max(dot(Normal, lightDir), 0.0) * Diffuse * vec3(1.0f) * .5f;
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
    //}

    FragColor = vec4(lighting, 1.0);
}
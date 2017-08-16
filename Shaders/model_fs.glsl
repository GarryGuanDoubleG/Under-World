#version 400 core
out vec4 color;

in vec2 UV;

in vec3 vs_normal;
in vec3 FragPos;

uniform vec3 viewPos;
uniform vec3 lightPos; 
uniform vec3 lightColor;
uniform vec3 colorMod;


uniform sampler2D texture_diffuse0;
uniform sampler2D texture_diffuse1;

void main()
{
	//color = vec4((texture(texture_diffuse1, UV)) *lightColor);
	vec3 lightDir = normalize(lightPos - FragPos); 
	
	float specularStrength = 0.69f;
	vec3 norm = normalize(vs_normal);
	vec3 viewDir = normalize(viewPos - FragPos);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
	vec3 specular = specularStrength * spec * lightColor;

	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * lightColor;
	
	float ambientStrength = 0.1f;
	vec3 ambient = ambientStrength * lightColor;

	vec4 model_color = vec4(texture(texture_diffuse0, UV));

    vec3 result = (specular + ambient + diffuse);
	//color = vec4(colorMod * result, 1.0f) * model_color;
	color = vec4(result, 1.0f) * model_color;
	color = color * vec4(colorMod, 1.0f);

}
#version 450 core
out vec4 color;

in vec2 UV;
in vec3 vs_normal;
in vec3 FragPos;

uniform vec3 viewPos;
uniform vec3 lightPos; 
uniform vec3 lightColor;

//struct Light{
//	vec3 position;
	
//	vec3 ambient;
//	vec3 diffuse;
//	vec3 specular;
//}

//uniform Light light;

uniform sampler2D textureDiffuse;
uniform sampler2D textureSpecular;

void main()
{
	vec3 lightDir = normalize(lightPos - FragPos); 
	
	//**********SPECULAR*************
	vec3 norm = normalize(vs_normal);
	vec3 viewDir = normalize(viewPos - FragPos);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 64);
	vec3 specular =  spec * lightColor * vec3(texture(textureSpecular, UV));

	//***********DIFFUSE*************
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * .5f * vec3(texture(textureDiffuse, UV));

	//***********AMBIENT*************
	float ambientStrength = 0.2f;
	vec3 ambient = 0.2f * lightColor * vec3(texture(textureDiffuse, UV));

    vec3 result = (specular + ambient + diffuse);
	color = vec4(result, 1.0f);
}
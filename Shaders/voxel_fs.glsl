#version 400 core
out vec4 color;

in vec3 vs_normal;
in vec3 FragPos;


uniform vec3 viewPos;
uniform vec3 lightPos; 
uniform vec3 lightColor;
uniform vec3 voxelColor;

uniform int chunkID;

void main(void)
{
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

    vec3 result = (specular + ambient + diffuse);
	//vec3 result = vec3(chunkID * 257  % 256 / 256.0,
	//					chunkID * 359 % 256 / 256.0,
	//					chunkID * 501 % 256 / 256.0 );

	vec3 chunkColor = vec3(chunkID * 257 % 256, chunkID * 359 % 256 , chunkID * 501 % 256) / 256;

	color = vec4(chunkColor, 1.0f);

	//float intensity =  500.0 - FragPos .z / 1000.0;
	//color = vec4(intensity, 1, intensity, 1.0);
}
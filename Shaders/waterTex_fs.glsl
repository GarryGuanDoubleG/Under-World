#version 400 core
out vec4 color;

in vec3 vs_normal;
in vec3 FragPos;
flat in int texID;

uniform vec3 viewPos;
uniform vec3 lightPos; 
uniform vec3 lightColor;
uniform vec3 voxelColor;

#define MAX_TEXTURES 15

uniform sampler2D voxelTexture[MAX_TEXTURES];


vec3 getTriPlanarBlend(vec3 _wNorm){
	// in wNorm is the world-space normal of the fragment
	vec3 blending = abs( _wNorm );
	blending = normalize(max(blending, 0.00001)); // Force weights to sum to 1.0
	float b = (blending.x + blending.y + blending.z);
	blending /= vec3(b, b, b);
	return blending;
}

void main(void)
{	
	int index = int(texID);
	//int index = 0;
	float scale = .05f;
	vec3 blending = getTriPlanarBlend(vs_normal);
	vec3 xaxis = texture2D( voxelTexture[index], FragPos.yz * scale).rgb;
	vec3 yaxis = texture2D( voxelTexture[index], FragPos.xz * scale).rgb;
	vec3 zaxis = texture2D( voxelTexture[index], FragPos.xy * scale).rgb;
	vec3 tex = xaxis * blending.x + yaxis * blending.y + zaxis * blending.z;


	vec3 lightDir = normalize(lightPos - FragPos); 
	
	float specularStrength = 0.8f;
	vec3 norm = normalize(vs_normal);

	vec3 viewDir = normalize(viewPos - FragPos);
	vec3 reflectDir = reflect(-lightDir, norm);
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(lightDir, halfwayDir), 0.0), 32);
	vec3 specular = specularStrength * spec * lightColor;

	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * lightColor;
	
	float ambientStrength = 0.3f;
	vec3 ambient = ambientStrength * lightColor;

    vec3 result = (specular + ambient + diffuse);

	//color =   vec4(tex * result, 1.0f);
	color =  vec4(tex * result, 1.0f);
}

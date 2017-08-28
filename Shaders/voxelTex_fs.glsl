#version 400 core
out vec4 color;

in vec3 vs_normal;
in vec3 FragPos;
flat in int texID;

uniform vec3 viewPos;
uniform vec3 lightDirection;
uniform vec3 lightColor;
uniform vec3 voxelColor;

#define MAX_TEXTURES 15

uniform sampler2D voxelTexture[MAX_TEXTURES];


vec3 getTriPlanarBlend(vec3 _wNorm){
	// in wNorm is the world-space normal of the fragment
	vec3 blending = abs( _wNorm );
	blending = normalize(max(blending, 0.00001)); // Force weights to sum to 1.0
	float b = (blending.x + blending.y + blending.z);
	blending /= vec3(b);
	return blending;
}

void main(void)
{	
	int index = int(texID);
	//int index = 0;
	float scale = .00015f;
	vec3 blending = getTriPlanarBlend(vs_normal);
	vec3 xaxis = texture2D( voxelTexture[index], FragPos.yz * scale).rgb;
	vec3 yaxis = texture2D( voxelTexture[index], FragPos.xz * scale).rgb;
	vec3 zaxis = texture2D( voxelTexture[index], FragPos.xy * scale).rgb;
	vec3 tex = xaxis * blending.x + yaxis * blending.y + zaxis * blending.z;

	//specular uses direction towards light source
	vec3 lightDir = normalize(-lightDirection);
	vec3 norm = normalize(vs_normal);
	vec3 viewDir = normalize(viewPos - FragPos);
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(viewDir, halfwayDir), 0.0), 32);
	vec3 specular =  .8f * spec * lightColor * tex;

	//diffuse uses direction away from light source
	float diff = max(dot(norm, -lightDir), 0.0);
	vec3 diffuse = diff * tex * lightColor * .5f;
	
	float ambientStrength = 0.2f;
	vec3 ambient = ambientStrength * lightColor * tex;

    vec3 result = (specular + ambient + diffuse);
	//vec3 result = diffuse;
	color = vec4(result, 1.0f);
}

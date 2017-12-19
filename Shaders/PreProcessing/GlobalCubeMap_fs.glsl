#version 450

out vec4 FragColor;

in vec3 localPos;

uniform samplerCube environmentMap;

void main()
{
	vec3 envColor = texture(environmentMap, localPos).rgb;
	
	//tonemap
	envColor = envColor / (envColor + vec3(1.0f));
	//gamma correct
	envColor = pow(envColor, vec3(1.0 / 2.2.));

	FragColor = envColor;
}

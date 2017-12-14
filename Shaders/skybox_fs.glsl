#version 450

out vec4 FragColor;
in vec3 FragPos;

uniform samplerCube environmentMap;

void main()
{
	vec3 envColor = texture(environmentMap, FragPos).rgb;
  
	FragColor = vec4(envColor, 1.0);
}

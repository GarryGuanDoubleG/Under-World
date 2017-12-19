#version 450

#pragma include "common.inc.glsl"

out vec4 FragColor;
in vec3 FragPos;

uniform samplerCube envMap;//environment map

void main()
{
	vec3 N = normalize(FragPos);
	vec3 irradiance = vec3(0.0);

	vec3 up = vec3(0.0, 1.0, 0.0);
	vec3 right = cross(up, N);
	up = cross(N, right);

	float sampleDelta = 0.025;
	float nrSamples = 0.0;

	for(float phi = 0.0; phi < M_PI * 2.0f; phi += sampleDelta)
	{
		for(float theta = 0.0; theta < M_PI * .5f; theta += sampleDelta)
		{
			vec3 tangentSample = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));

			vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N;

			irradiance += textureLod(envMap, sampleVec, 0).rgb * cos(theta) * sin(theta);
			nrSamples++;
		}
	}
	irradiance = M_PI * irradiance * (1.0 / float(nrSamples));

	FragColor = vec4(irradiance, 1.0f);
}
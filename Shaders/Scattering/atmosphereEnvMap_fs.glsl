#version 450 core

out vec4 result;

in vec3 FragPos;

#pragma include "common.inc.glsl"

layout(binding = 0) uniform sampler3D texInscatter;

//const float sunIntensity = 30.0f;
uniform float sunIntensity;

//HDR Tone Map 
const float EXPOSURE = 1.f;
const vec3 viewPos = vec3(0.0f, 1e4, 0.0);

#pragma include "scatterFunctions.inc.glsl"
#pragma include "compute_scattering.inc.glsl"
 

vec3 HDR(vec3 color)
{
	return 1.0f - exp(-EXPOSURE * color);
}

void main()
{
	vec3 viewDir = normalize(FragPos);
	vec3 surfacePos = FragPos * VIEW_DISTANCE;

	vec3 attenutation = vec3(1.f);
	float irradianceFactor = 0.0f;
	float sky_clip = 0.0;

	vec3 inscatterLight = DoScattering(surfacePos, viewDir, sky_clip) * sunIntensity * 2;

	inscatterLight *= M_PI;
	vec3 silhouette_col = vec3(sunIntensity) * inscatterLight;

	silhouette_col *= 2.0;
	float disk_factor = pow(clamp(dot(viewDir, -sunDir) + 0.0069, 0, 1), 23.0 * 1e5);
	float upper_disk_factor = smoothstep(0, 1, (viewDir.y + 45.f) * 1.0);
	inscatterLight += vec3(1, 0.1, 0.3) * disk_factor * upper_disk_factor * silhouette_col * 3.0 * 1e3;
	//tone map
	result = vec4(HDR(inscatterLight), 1.0f);
	//result = vec4(vec3(1.0f), 1.0f);
	//result = vec4(FragPos, 1.0f);
}
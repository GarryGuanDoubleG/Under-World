#version 430 core

out vec4 result;

in vec3 FragPos;
in vec2 UV;
in vec3 viewPos;

layout (binding = 0) uniform sampler2D texIrradiance;
layout (binding = 1) uniform sampler2D texTransmittance;
layout (binding = 2) uniform sampler3D texInscatter;

layout (binding = 3) uniform sampler2D gPosition;
layout (binding = 4) uniform sampler2D gNormal;
layout (binding = 5) uniform sampler2D gAlbedoSpec;

layout (binding = 6) uniform sampler2D ShadedScene;
//const float sunIntensity = 30.0f;
uniform float sunIntensity;

#pragma include "common.inc.glsl"
#pragma include "scatterFunctions.inc.glsl"
#pragma include "compute_scattering.inc.glsl"

const float EXPOSURE = 1.f;

vec3 HDR(vec3 color)
{
	return 1.0f - exp(-EXPOSURE * color);
}

void main()
{	
	vec3 surfacePos = texture(gPosition, UV).rgb;
	vec3 viewDir = normalize(surfacePos - viewPos);
	vec3 normal = texture(gNormal, UV).rgb;
	vec3 diffuse = texture(gAlbedoSpec, UV).rgb;
	float Specular = texture(gAlbedoSpec, UV).a;

	//viewDir.y *= -1;

	vec3 attenutation = vec3(1.f);
	float irradianceFactor = 0.0f;
	float sky_clip = 0.0;

	vec3 inscatterLight = DoScattering(surfacePos, viewDir, sky_clip) * sunIntensity;


	if(is_skybox(surfacePos))
	{
		inscatterLight *= M_PI;
		vec3 silhouette_col = vec3(sunIntensity) * inscatterLight * sky_clip;

        silhouette_col *= 2.0;
        float disk_factor = pow(clamp(dot(viewDir, sunDir) + 0.00069, 0, 1), 23.0 * 1e5);		
        float upper_disk_factor = smoothstep(0, 1, (viewDir.y + 0.045) * 1.0);
        inscatterLight += vec3(1, 0.1, 0.3) * disk_factor * upper_disk_factor * silhouette_col * 3.0 * 1e3;
	}
	else
	{
		//inscatterLight *= 10.0f;
		float dist = distance(surfacePos, viewPos);
		float scatterExtinction = 2000;
		float extinction = clamp((dist / scatterExtinction * 10.0f), 0, 1);
		inscatterLight *= extinction;
	}

	result.xyz = textureLod(ShadedScene, UV, 0).xyz;
	result.w = 1.0f;

	vec3 tone_map = result.xyz + inscatterLight;
	tone_map = HDR(tone_map);

	result.xyz = tone_map;
}

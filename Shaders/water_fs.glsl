#version 400 core

in vec4 clipSpace;
in vec2 dudvCoords;
in vec3 cameraVector;


out vec4 color;

uniform sampler2D reflectionTex;
uniform sampler2D refractionTex;
uniform sampler2D dudvMap;

uniform float moveSpeed;

const float waveStrength = 0.1f;

void main(void)
{
	vec2 ndc = (clipSpace.xy / clipSpace.w) / 2.0 + 0.5;

	vec2 refractUV = ndc;
	vec2 reflectUV = vec2(ndc.x, -ndc.y);
	

	//vec2 distortion1 = (texture(dudvMap, vec2(dudvCoords.x + moveSpeed, dudvCoords.y)).rg * 2.0 - 1.0) * waveStrength;
	//vec2 distortion2 = (texture(dudvMap, vec2(-dudvCoords.y + moveSpeed, dudvCoords.y + moveSpeed)).rg * 2.0 - 1.0) * waveStrength;

	//vec2 totalDistort = distortion1 + distortion2;

	//refractUV += totalDistort;
	//reflectUV += totalDistort;

	//reflectUV.x = clamp(reflectUV.x, 0.001, 0.999);
	//reflectUV.y = clamp(reflectUV.y, -0.999, -0.001);

	vec4 reflectColor = texture(reflectionTex, reflectUV);
	vec4 refractColor = texture(refractionTex, refractUV);

	//vec3 viewVector = normalize(cameraVector);
	//float refractionFactor = dot(viewVector,  vec3(0.0,1.0, 0.0));
	//refractionFactor = (refractionFactor + 1) / 2.0f;

	color = mix(reflectColor, refractColor, .15f);
	color.a = 1.0f;
	//color = mix(color, vec4(0.0, 0.3, 0.5, 1.0),  0.2);
}
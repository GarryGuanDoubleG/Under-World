#version 400 core

in vec3 UV;
out vec4 color;

uniform samplerCube daySkybox;
uniform samplerCube nightSkybox;

uniform float blendFactor;

void main()
{    
	vec4 dayColor = texture(daySkybox, UV);
	vec4 nightColor = texture(nightSkybox, UV);

	vec4 final = mix(dayColor, nightColor, blendFactor);

    color = final;
}
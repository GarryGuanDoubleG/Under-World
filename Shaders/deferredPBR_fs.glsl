#version 450 core
layout (location = 0) out vec3 outPosition;
layout (location = 1) out vec3 outNormal;
layout (location = 2) out vec3 outAlbedo;
layout (location = 3) out float outMetallic;
layout (location = 4) out float outRoughness;

in vec2 UV;
in vec3 FragPos;
in vec3 Normal;

uniform sampler2D albedo;
uniform sampler2D metallic;
uniform sampler2D roughness;

void main()
{    
    // store the fragment position vector in the first gbuffer texture
    outPosition = FragPos;
    
	// also store the per-fragment normals into the gbuffer
    outNormal = Normal;

    // and the diffuse per-fragment color with gamma correction
    outAlbedo = pow(texture(albedo, UV).rgb, vec3(2.2));
    
	//metallic component
    outMetallic = texture(metallic, UV).r;

	//roughness
	outRoughness = texture(roughness, UV).r;
}
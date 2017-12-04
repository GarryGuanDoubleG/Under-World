#version 450 core
layout (location = 0) out vec3 outPosition;
layout (location = 1) out vec3 outNormal;
layout (location = 2) out vec3 outAlbedo;
layout (location = 3) out float outAO;
layout (location = 4) out float outMetallic;
layout (location = 5) out float outRoughness;

in vec2 UV;
in vec3 FragPos;
in vec3 Normal;
in mat3 TBN;

uniform sampler2D albedo;
uniform sampler2D AO;
uniform sampler2D normalMap;
uniform sampler2D metallic;
uniform sampler2D roughness;

void main()
{    
    // store the fragment position vector in the first gbuffer texture
    outPosition = FragPos;
    
	// also store the per-fragment normals into the gbuffer
	vec3 normal = texture(normalMap, UV).rgb;
	normal = normal * 2.0 - 1.0;   
	normal = normalize(TBN * normal); 

    outNormal = normal;

    // and the diffuse per-fragment color with gamma correction
    outAlbedo = pow(texture(albedo, UV).rgb, vec3(2.2));
    
	//baked ao component
	outAO = texture(AO, UV).r;

	//metallic component
    outMetallic = texture(metallic, UV).r;

	//roughness
	outRoughness = texture(roughness, UV).r;
}
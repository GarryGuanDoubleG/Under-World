#version 450 core

#pragma include "common.inc.glsl"
#pragma include "voxelCommon.inc.glsl"

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;

in VS_OUT
{
	vec3 FragPos;
	vec3 normal;
	flat int texID;
}fs_in;

uniform mat4 model;

void main()
{    
	int index = int(fs_in.texID);
	
	//calculate texture value at this fragment
	float scale = .0015f;
	vec3 blending = getTriPlanarBlend(fs_in.normal);
	//vec3 blending = sunDir;
    // store the fragment position vector in the first gbuffer texture
    gPosition = fs_in.FragPos;
    // also store the per-fragment normals into the gbuffer
    gNormal = TriPlanarNormal(fs_in.FragPos, fs_in.normal, blending, scale);
	//gNormal = transpose(inverse(mat3(model))) * gNormal;
    // and the diffuse per-fragment color
    gAlbedoSpec.rgb = GetTriPlanarTex(fs_in.FragPos, blending, scale, index).rgb;
	//gAlbedoSpec = vec4(.7f, .7f, .7f, 1.0f);
	//gAlbedoSpec *= 0;
	//gAlbedoSpec.rgb += vec3(.5f);
    // store specular intensity in gAlbedoSpec's alpha component
    gAlbedoSpec.a =  .75f;
}
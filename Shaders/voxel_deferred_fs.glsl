#version 450 core

#pragma include "common.inc.glsl"
#pragma include "voxelCommon.inc.glsl"

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;

in VS_OUT
{
	vec3 ScreenFragPos;
	vec3 WorldFragPos;
	vec3 normal;
	flat int texID;
}fs_in;

void main()
{    
	int index = int(fs_in.texID);
	
	//calculate texture value at this fragment
	float scale = .0016f;

	vec3 blending = getTriPlanarBlend(fs_in.normal);

    gPosition = fs_in.ScreenFragPos;
    // also store the per-fragment normals into the gbuffer
    gNormal = TriPlanarNormal(fs_in.WorldFragPos, fs_in.normal, blending, scale);
	//gNormal = fs_in.normal;

    // and the diffuse per-fragment color
    gAlbedoSpec.rgb = GetTriPlanarTex(fs_in.WorldFragPos, blending, scale, index).rgb;

    // store specular intensity in gAlbedoSpec's alpha component
    gAlbedoSpec.a = .75f;
}
#version 430 core

// Shader to generate the cloud noise textures

layout (local_size_x = 8, local_size_y = 8, local_size_z = 4) in;
uniform writeonly image3D DestTex;

#pragma include "common.inc.glsl"
#pragma include "noise.inc.glsl"

void main() {
	// The single most important value to get right. All other values should be hardcoded,
	// but it would be nice to have this one exposed as a [0,1] slider. It controls how 
	// fluffy (perlin) or billowy (worley) the clouds look
	float perlin_to_worley_ratio    =  0.3;
	// Texture 2
	float texture2_low              = -0.2;
	float texture2_high             =  1.0;

	ivec3 coord = ivec3(gl_GlobalInvocationID.xyz);
	vec3 fragCoord = vec3(coord) / imageSize(DestTex).xyz;

	// Build the lower resolution worley noise of each channel of the first 3d texture. Each stage is
	// has it's values remmaped to a range which exploits the RGBA8 efficiently
	float worley_value_r = get_worley_3_octaves(fragCoord, 10);
	float worley_value_g = get_worley_3_octaves(fragCoord, 15);
	float worley_value_b = get_worley_3_octaves(fragCoord, 20);
	float worley_value_a = get_worley_3_octaves(fragCoord, 10);

	// Remap the values
	worley_value_r = set_range(worley_value_r, texture2_low, texture2_high);
	worley_value_g = set_range(worley_value_g, texture2_low, texture2_high);
	worley_value_b = set_range(worley_value_b, texture2_low, texture2_high);
	worley_value_a = set_range(worley_value_a, texture2_low, texture2_high);
	
	imageStore(DestTex, coord, vec4(worley_value_r, worley_value_g, worley_value_b, worley_value_a));
}

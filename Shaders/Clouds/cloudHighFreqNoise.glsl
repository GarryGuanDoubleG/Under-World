#version 430 core


// Shader to generate the cloud noise textures

layout (local_size_x = 8, local_size_y = 8, local_size_z = 4) in;
uniform writeonly image3D DestTex;

const int tex_size = 128;

#pragma include "common.inc.glsl"
#pragma include "noise.inc.glsl"

void main() 
{
	// The single most important value to get right. All other values should be hardcoded,
	// but it would be nice to have this one exposed as a [0,1] slider. It controls how 
	// fluffy (perlin) or billowy (worley) the clouds look
	float perlin_to_worley_ratio    =  0.3;

	// Texture 1
	float texture1_r_perlin_low     =  0.3;
	float texture1_r_perlin_high    =  1.4;
	float texture1_r_worley_low     = -0.3;
	float texture1_r_worley_high    =  1.3;
	float texture1_gba_worley_low   = -0.4;
	float texture1_gba_worley_high  =  1.0;


	ivec3 coord = ivec3(gl_GlobalInvocationID.xyz);
	vec3 fragCoord = vec3(coord) / imageSize(DestTex).xyz;

	// Build the perlin and worley noise of each channel of the first 3d texture. Each stage is
	// has it's values remmaped to a range which exploits the RGBA8 efficiently
	float perlin_r = get_perlin_7_octaves(fragCoord, 4.0);
	float worley_r = get_worley_3_octaves(fragCoord, 6.0);
	float worley_g = get_worley_3_octaves(fragCoord, 6.0);
	float worley_b = get_worley_3_octaves(fragCoord, 12.0);
	float worley_a = get_worley_3_octaves(fragCoord, 24.0);

	// Remap the values
	perlin_r = set_range(perlin_r, texture1_r_perlin_low, texture1_r_perlin_high);
	worley_r = set_range(worley_r, texture1_r_worley_low, texture1_r_worley_high);
	worley_g = set_range(worley_g, texture1_gba_worley_low, texture1_gba_worley_high);
	worley_b = set_range(worley_b, texture1_gba_worley_low, texture1_gba_worley_high);
	worley_a = set_range(worley_a, texture1_gba_worley_low, texture1_gba_worley_high);

	// Combinning the two noises (this is what they refer as "dilating" the perlin noise)
	float worley_perlin = dilate_perlin_worley(perlin_r, worley_r, perlin_to_worley_ratio);
	
	imageStore(DestTex, coord, vec4(worley_perlin, 1.0-worley_g, 1.0-worley_b, 1.0-worley_a));
}

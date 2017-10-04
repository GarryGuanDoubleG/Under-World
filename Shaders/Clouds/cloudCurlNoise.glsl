#version 430 core

// Shader to generate the cloud noise textures

layout (local_size_x = 8, local_size_y = 8) in;
uniform writeonly image2D DestTex;


#pragma include "common.inc.glsl"
#pragma include "cloud_common.inc.glsl"
#pragma include "noise.inc.glsl"

const float curl_scale                =  3.0;
const float curl_low				  = -0.5;
const float curl_high                 =  3.0;

void main() {
	ivec2 coord = ivec2(gl_GlobalInvocationID.xy);
	vec3 fragCoord =  vec3(vec2(coord) / vec2(imageSize(DestTex).xy), 0);

	vec3 curlValues = curl_noise(fragCoord * 3);
	curlValues = set_ranges_signed(curlValues, curl_low, curl_high);

	imageStore(DestTex, coord, vec4(encode_curl(curlValues), 0));
	//imageStore(DestTex, coord, vec4(1.0f, 0,0,1));
}

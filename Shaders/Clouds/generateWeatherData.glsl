#version 450 core

layout (local_size_x = 8, local_size_y = 8, local_size_z = 4) in;

#pragma include "common.inc.glsl"
#pragma include "cloud_common.inc.glsl"
#pragma include "noise.inc.glsl"

uniform sampler2D cloudWeatherMap;

const float clouds_advance_coverage_speed = 2.0;
const float clouds_advance_type_speed = 2.0;
const float clouds_advance_wetness_speed = 1.0;
const float clouds_coverage = 0.35;
const float clouds_type = .5;
const float clouds_wetness  = 0.0;
const float clouds_wind_speed = 2.0f;
const float clouds_time = 0.0f;

uniform writeonly image2D DestTex;

void main()
{
	ivec2 coord = ivec2(gl_GlobalInvocationID.xy);
	vec2 uv = vec2(coord.xy) / vec2(imageSize(DestTex).xy);

	vec2 xy_offset  = vec2(get_time(clouds_time, clouds_wind_speed * WEATHER_TIME_SCALAR) + clouds_time);
	//vec2 xy_offset = vec2(1.0f);

 	float speed_scale = clouds_wind_speed;

	vec2 xy_offset1 = xy_offset * clouds_advance_coverage_speed * speed_scale;
	vec2 xy_offset2 = xy_offset * clouds_advance_type_speed * speed_scale;
	vec2 xy_offset3 = xy_offset * clouds_advance_wetness_speed * speed_scale;

	float z_offset1 = 0.0;
	float z_offset2 = 500.0;
	float z_offset3 = 100.0;
	float z_offset4 = 200.0;

	vec3 sampling_pos1 = vec3(uv + xy_offset1, z_offset1) * 2.0;
	vec3 sampling_pos2 = vec3(uv + xy_offset2, z_offset2) * 4.0;
	vec3 sampling_pos3 = vec3(uv + xy_offset2, z_offset3) * 5.5;
	vec3 sampling_pos4 = vec3(uv + xy_offset2, z_offset4) * 2.0;

	float signal1 = get_perlin_5_octaves(sampling_pos1, false);
	float signal2 = get_perlin_5_octaves(sampling_pos2, false);
	float signal3 = get_perlin_5_octaves(sampling_pos3, false);
	float signal4 = get_perlin_5_octaves(sampling_pos4, false);
	 				
	float perlin1 = set_range(signal1, 0.5, 1.3);
	float perlin2 = set_range(signal2, 0.5, 1.3);
	float perlin3 = set_range(signal3, 0.5, 1.3);
	float perlin4 = set_range(signal4, 0.5, 1.3);

	perlin1 = pow(perlin1, 0.75);
	perlin2 = pow(perlin2, 1.00);
	perlin3 = pow(perlin3, 1.00);
	perlin4 = pow(perlin4, 0.50);

	perlin1 = saturate(perlin1 * 1.2) * 0.4 + 0.1;
	perlin2 = saturate(perlin2 * 1.0) * 0.5;
	perlin3 = saturate(perlin3 * 1.0) * 0.5;
	perlin4 = saturate(1.0 - perlin4 * 2.0);

	perlin1 -= perlin4 * 0.5;
	perlin2 += pow(perlin4, 2.0);
	perlin3 += perlin4;

	imageStore(DestTex, coord, vec4(perlin1, perlin2, perlin3, perlin4));
	//imageStore(DestTex, coord, vec4(0));
}

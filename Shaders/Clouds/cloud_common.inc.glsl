#define DEBUG_WIN_SIZE 256
#define TEXTURE1_RES 256
#define TEXTURE2_RES 64
#define TEXTURE3_RES 128

// The (inverted) resolution scale at which we update the clouds pixels
#define LOW_RES_FACTOR 4
#define LOW_RES_FACTOR_X_TWO 16

#define EARTH_RADIUS 150000.0
#define EARTH_CENTER vec3(0, 0, -EARTH_RADIUS)
#define CLOUDS_START 1500.0
#define CLOUDS_END 5000.0
#define MAX_CLOUD_DISTANCE 12000.0
						
#define NUM_STEPS 256
#define NUM_GODRAY_SAMPLES 32
#define LIGHTING_SAMPLES_1TO5_STEP_SIZE 60.0
#define LIGHTING_SAMPLES_6_STEP_SIZE 800.0
#define SKY_CLOUDS_MERGE_TONEMAP_RANGE 0.1

#define CLOUD_DENSITY_THRESHOLD 0.001
#define COVERAGE_SCALAR 1.2
#define COVERAGE_OFFSET -0.3

// The time scalar to use accross all clouds calculations and
// weather scalar to align weather speed with cloud speed
#define TIME_SCALAR 500.0 
#define TIME_OFFSET_SCALAR TIME_SCALAR * 10.0 
#define WEATHER_TIME_SCALAR 0.000013
#define UP_DOWN_DRAFT_RANGE 8000.0

#define SUN_DISTANCE 10000000.0


uniform float time;

ivec2 reprojection_offsets[16] = {
	ivec2(2,1), ivec2(1,2), ivec2(2,0), ivec2(0,1),
	ivec2(2,3), ivec2(3,2), ivec2(3,1), ivec2(0,3),
	ivec2(1,0), ivec2(1,1), ivec2(3,3), ivec2(0,0),
	ivec2(2,2), ivec2(1,3), ivec2(3,0), ivec2(0,2)
};

vec2 gradient_stratocumulus = vec2(0.1, 0.3);
vec2 gradient_cumulus = vec2(0.2, 1.0);
vec2 gradient_cumulonimbus = vec2(0.75, 1.0);

float get_time(float cloud_time, float cloud_wind_speed) {
	return time * cloud_wind_speed * TIME_SCALAR;
}

vec3 encode_curl(vec3 c) {
	return (c + 1.0) * 0.5;
}

vec3 decode_curl(vec3 c) {
	return (c - 0.5) * 2.0;
}

//vec3 get_raymarch_direction(vec3 sun_dir){
//	float direction = get_night_time(sun_dir, 3.0, -0.3) > 0.0 ? -1.0 : 1.0;
//	return sun_dir * direction;
//}
#define saturate(v) clamp(v, 0, 1)

#define M_PI 3.1415926535897932384626433
#define HALF_PI 1.5707963267948966192313216
#define TWO_PI 6.2831853071795864769252867
#define FOUR_PI 12.566370614359172953850573
#define ONE_BY_PI 0.3183098861837906715377675
#define SQRT_TWO 1.4142135623730950488016887

#define WINDOW_WIDTH 1680
#define WINDOW_HEIGHT 1080

#define SCREEN_SIZE vec2(WINDOW_WIDTH, WINDOW_HEIGHT)

#define get_half_texcoord() vec2((ivec2(gl_FragCoord.xy) * 2 + 0.5) / SCREEN_SIZE)

// Returns x * x
float square(float x) { return x * x; }
vec2 square(vec2 x) { return x * x; }
vec3 square(vec3 x) { return x * x; }
vec4 square(vec4 x) { return x * x; }

//const float sun_azimuth = 0;
//const float sun_altitude = 1;
uniform float sun_azimuth;
uniform float sun_altitude;
uniform vec3 sunDir;

// Converts a normalized spherical coordinate (r = 1) to cartesian coordinates
vec3 spherical_to_vector(float theta, float phi) {
    float sin_theta = sin(theta);
    return normalize(vec3(
        sin(phi) * cos(theta),
		cos(phi),
        sin_theta * sin(phi)
    ));
}

// Converts a given sun azimuth and altitude to a direction vector
vec3 sun_azimuth_to_angle(float azimuth, float altitude) {
    float phi = (90 - altitude) / 180.0 * M_PI;
    float theta = azimuth / 180.0 * M_PI;
    return spherical_to_vector(theta, phi);
	//return spherical_to_vector(azimuth, altitude);
	//return sunDir;
}

//#define get_sun_vector() sun_azimuth_to_angle(sun_azimuth, sun_altitude);
#define get_sun_vector() sunDir
#define sun_vector sunDir
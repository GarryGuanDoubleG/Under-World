
vec3 interpolation_c2( vec3 x ) { 
	return x * x * x * (x * (x * 6.0 - 15.0) + 10.0); 
}

float set_range(float value, float low, float high) 
{
	return saturate((value - low)/(high - low));
}

vec3 set_ranges_signed(vec3 values, float low, float high) {
	return (values - low)/(high - low);
}

float rand(vec2 co){
  return abs(fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453)) * 2 - 1;
}

float dilate_perlin_worley(float p, float w, float x) {
	float curve = 0.75;
	if(x < 0.5) {
		x = x/0.5;
		float n = p + w * x;
		return n * mix(1.f, 0.5, pow(x, curve));
	} else {
		x = (x-0.5)/0.5;
		float n = w + p * (1.0 - x);
		return n * mix(0.5, 1.0, pow(x, 1.0/curve));
	}
}

// from: https://github.com/BrianSharpe/GPU-Noise-Lib/blob/master/gpu_noise_lib.glsl
void perlin_hash(vec3 gridcell, float s, bool tile, 
					out vec4 lowz_hash_0,
					out vec4 lowz_hash_1,
					out vec4 lowz_hash_2,
					out vec4 highz_hash_0,
					out vec4 highz_hash_1,
					out vec4 highz_hash_2)
{
	const vec2 OFFSET = vec2( 50.0, 161.0 );
	const float DOMAIN = 69.0;
	const vec3 SOMELARGEFLOATS = vec3( 635.298681, 682.357502, 668.926525 );
	const vec3 ZINC = vec3( 48.500388, 65.294118, 63.934599 );

	gridcell.xyz = gridcell.xyz - floor(gridcell.xyz * ( 1.0 / DOMAIN )) * DOMAIN;
	float d = DOMAIN - 1.5;
	vec3 gridcell_inc1 = step( gridcell, vec3( d,d,d ) ) * ( gridcell + 1.0 );

	gridcell_inc1 = tile ? mod(gridcell_inc1, s) : gridcell_inc1;
	

	vec4 P = vec4( gridcell.xy, gridcell_inc1.xy ) + OFFSET.xyxy;
	P *= P;
	P = P.xzxz * P.yyww;
	vec3 lowz_mod = vec3( 1.0 / ( SOMELARGEFLOATS.xyz + gridcell.zzz * ZINC.xyz ) );
	vec3 highz_mod = vec3( 1.0 / ( SOMELARGEFLOATS.xyz + gridcell_inc1.zzz * ZINC.xyz ) );
	lowz_hash_0 = fract( P * lowz_mod.xxxx );
	highz_hash_0 = fract( P * highz_mod.xxxx );
	lowz_hash_1 = fract( P * lowz_mod.yyyy );
	highz_hash_1 = fract( P * highz_mod.yyyy );
	lowz_hash_2 = fract( P * lowz_mod.zzzz );
	highz_hash_2 = fract( P * highz_mod.zzzz );
}

// from: https://github.com/BrianSharpe/GPU-Noise-Lib/blob/master/gpu_noise_lib.glsl
float perlin(vec3 P, float s, bool tile) {
	P *= s;

	vec3 Pi = floor(P);
	vec3 Pi2 = floor(P);
	vec3 Pf = P - Pi;
	vec3 Pf_min1 = Pf - 1.0;

	vec4 hashx0, hashy0, hashz0, hashx1, hashy1, hashz1;
	perlin_hash( Pi2, s, tile, hashx0, hashy0, hashz0, hashx1, hashy1, hashz1 );

	vec4 grad_x0 = hashx0 - 0.49999;
	vec4 grad_y0 = hashy0 - 0.49999;
	vec4 grad_z0 = hashz0 - 0.49999;
	vec4 grad_x1 = hashx1 - 0.49999;
	vec4 grad_y1 = hashy1 - 0.49999;
	vec4 grad_z1 = hashz1 - 0.49999;
	vec4 grad_results_0 = inversesqrt( grad_x0 * grad_x0 + grad_y0 * grad_y0 + grad_z0 * grad_z0 ) * ( vec2( Pf.x, Pf_min1.x ).xyxy * grad_x0 + vec2( Pf.y, Pf_min1.y ).xxyy * grad_y0 + Pf.zzzz * grad_z0 );
	vec4 grad_results_1 = inversesqrt( grad_x1 * grad_x1 + grad_y1 * grad_y1 + grad_z1 * grad_z1 ) * ( vec2( Pf.x, Pf_min1.x ).xyxy * grad_x1 + vec2( Pf.y, Pf_min1.y ).xxyy * grad_y1 + Pf_min1.zzzz * grad_z1 );

	vec3 blend = interpolation_c2( Pf );
	vec4 res0 = mix( grad_results_0, grad_results_1, blend.z );
	vec4 blend2 = vec4( blend.xy, vec2( 1.0 - blend.xy ) );
	float final = dot( res0, blend2.zxzx * blend2.wwyy );
	final *= 1.0/sqrt(0.75);
	return ((final * 1.5) + 1.0) * 0.5;
}

float perlin(vec3 P) {
	return perlin(P, 1, false);
}

vec3 voronoi_hash( vec3 x, float s) {
	x = mod(x, s);
	x = vec3( dot(x, vec3(127.1,311.7, 74.7)),
				dot(x, vec3(269.5,183.3,246.1)),
				dot(x, vec3(113.5,271.9,124.6)));
				
	return fract(sin(x) * 43758.5453123);
}

vec3 voronoi( in vec3 x, float s, bool inverted) {
	x *= s;
	x += 0.5;
	vec3 p = floor(x);
	vec3 f = fract(x);

	float id = 0.0;
	vec2 res = vec2( 1.0 , 1.0);
	for(int k=-1; k<=1; k++){
		for(int j=-1; j<=1; j++) {
			for(int i=-1; i<=1; i++) {
				vec3 b = vec3(i, j, k);
				vec3 r = vec3(b) - f + voronoi_hash(p + b, s);
				float d = dot(r, r);

				if(d < res.x) {
					id = dot(p + b, vec3(1.0, 57.0, 113.0));
					res = vec2(d, res.x);			
				} else if(d < res.y) {
					res.y = d;
				}
			}
		}
	}

	vec2 result = res;//sqrt(res);
	id = abs(id);

	if(inverted)
		return vec3(1.0 - result, id);
	else
		return vec3(result, id);
}

float get_worley_2_octaves(vec3 p, vec3 offset) {
	vec3 xyz = p + offset;

	float worley_value1 = voronoi(xyz, 1.0, true).r;
	float worley_value2 = voronoi(xyz, 2.0, false).r;

	worley_value1 = saturate(worley_value1);
	worley_value2 = saturate(worley_value2);

	float worley_value = worley_value1;
	worley_value = worley_value - worley_value2 * 0.25;

	return worley_value;;
}

float get_worley_3_octaves(vec3 p, float s) {
	vec3 xyz = p;

	float worley_value1 = voronoi(xyz, 1.0 * s, true).r;
	float worley_value2 = voronoi(xyz, 2.0 * s, false).r;
	float worley_value3 = voronoi(xyz, 4.0 * s, false).r;

	worley_value1 = saturate(worley_value1);
	worley_value2 = saturate(worley_value2);
	worley_value3 = saturate(worley_value3);

	float worley_value = worley_value1;
	worley_value = worley_value - worley_value2 * 0.3;
	worley_value = worley_value - worley_value3 * 0.3;

	return worley_value;;
}

float get_perlin_5_octaves(vec3 p, bool tile) {
	vec3 xyz = p;
	float amplitude_factor = 0.5;
	float frequency_factor = 2.0;

	float a = 1.0;
	float perlin_value = 0.0;
	perlin_value += a * perlin(xyz).r; a *= amplitude_factor; xyz *= (frequency_factor + 0.02);
	perlin_value += a * perlin(xyz).r; a *= amplitude_factor; xyz *= (frequency_factor + 0.03);
	perlin_value += a * perlin(xyz).r; a *= amplitude_factor; xyz *= (frequency_factor + 0.01);
	perlin_value += a * perlin(xyz).r; a *= amplitude_factor; xyz *= (frequency_factor + 0.01);
	perlin_value += a * perlin(xyz).r;

	return perlin_value;
}

float get_perlin_7_octaves(vec3 p, float s) {
	vec3 xyz = p;
	float f = 1.0;
	float a = 1.0;

	float perlin_value = 0.0;
	perlin_value += a * perlin(xyz, s * f, true).r; a *= 0.5; f *= 2.0;
	perlin_value += a * perlin(xyz, s * f, true).r; a *= 0.5; f *= 2.0;
	perlin_value += a * perlin(xyz, s * f, true).r; a *= 0.5; f *= 2.0;
	perlin_value += a * perlin(xyz, s * f, true).r; a *= 0.5; f *= 2.0;
	perlin_value += a * perlin(xyz, s * f, true).r; a *= 0.5; f *= 2.0;
	perlin_value += a * perlin(xyz, s * f, true).r; a *= 0.5; f *= 2.0;
	perlin_value += a * perlin(xyz, s * f, true).r;

	return perlin_value;
}

vec3 curl_noise(vec3 pos) {
	float e = 0.05;
	float n1, n2, a, b;
	vec3 c;

	n1 = get_perlin_5_octaves(pos.xyz + vec3( 0, e, 0), true);
	n2 = get_perlin_5_octaves(pos.xyz + vec3( 0,-e, 0), true);
	a = (n1-n2)/(2*e);
	n1 = get_perlin_5_octaves(pos.xyz + vec3( 0, 0, e), true);
	n2 = get_perlin_5_octaves(pos.xyz + vec3( 0, 0,-e), true);
	b = (n1-n2)/(2*e);

	c.x = a - b;

	n1 = get_perlin_5_octaves(pos.xyz + vec3( 0, 0, e), true);
	n2 = get_perlin_5_octaves(pos.xyz + vec3( 0, 0,-e), true);
	a = (n1-n2)/(2*e);
	n1 = get_perlin_5_octaves(pos.xyz + vec3( e, 0, 0), true);
	n2 = get_perlin_5_octaves(pos.xyz + vec3(-e, 0, 0), true);
	b = (n1-n2)/(2*e);

	c.y = a - b;

	n1 = get_perlin_5_octaves(pos.xyz + vec3( e, 0, 0), false);
	n2 = get_perlin_5_octaves(pos.xyz + vec3(-e, 0, 0), false);
	a = (n1-n2)/(2*e);
	n1 = get_perlin_5_octaves(pos.xyz + vec3( 0, e, 0), false);
	n2 = get_perlin_5_octaves(pos.xyz + vec3( 0,-e, 0), false);
	b = (n1-n2)/(2*e);

	c.z = a - b;

	return c;
}
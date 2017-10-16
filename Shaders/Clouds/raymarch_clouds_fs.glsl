#version 450 core

#pragma include "common.inc.glsl"
#pragma include "noise.inc.glsl" // 295

out vec4 result;

in vec2 UV;
in vec3 viewPos;

//g buffer
layout (binding = 0) uniform sampler2D gPosition;

layout (binding = 1) uniform sampler3D _WorleyNoise;
layout (binding = 2) uniform sampler3D _PerlinWorleyNoise;
layout (binding = 3) uniform sampler2D _CurlNoise;

layout(binding = 4) uniform sampler2D _WeatherTexture;

const float atmosphereEndHeight = 7500;
const float atmosphereStartHeight = 4500;
const float atmosphereThickness = atmosphereEndHeight - atmosphereStartHeight;
const float horizonDistance = 35000;
//const float _EarthRadius = CalculatePlanetRadius(atmosphereStartHeight, horizonDistance);//407583.3
const float _EarthRadius = 6371000.0f;

//const float maxDistance = CalculateMaxDistance(_EarthRadius, atmosphereEndHeight);//57242
const float maxDistance = 70242.f;
//const float maxRayDistance = CalculateMaxRayDistance(_EarthRadius, atmosphereStartHeight, atmosphereEndHeight);
const float maxRayDistance = 22242.0f;
const float _StartHeight = atmosphereStartHeight;
const float _EndHeight = atmosphereEndHeight;
const float _BaseFBMScale = 2;
const float _DetailScale = 4;
const float _DetailFBMScale = .22f;
const float _AtmosphereThickness = atmosphereThickness;

const float _MaxDistance = maxDistance;

const float _FieldOfView = 60.0f;
const float _AspectRatio = ASPECT_RATIO;


const float sunRayLength = 0.08f * atmosphereThickness;
const float coneRadius = 0.08f * atmosphereThickness;

float _CloudBottomFade = 0.33f;
			
//vec3 _CameraPosition = vec3(0, _EarthRadius, 0) + viewPos.rgb;
vec3 _CameraPosition = vec3(viewPos.x, _EarthRadius, viewPos.z);
//vec3 _CameraPosition = viewPos;

//animations
uniform vec3 _BaseOffset ;
uniform vec3 _DetailOffset ;
uniform vec2 _CoverageOffset;


float _BaseScale = 1.f / (atmosphereEndHeight);
float _CoverageScale = 1.0f / (maxDistance * 10);
float _HorizonFadeStartAlpha = .5f;
float _OneMinusHorizonFadeStartAlpha = 1.0f - _HorizonFadeStartAlpha;
float _HorizonFadeScalar = .1f;					// Fades clouds on horizon, 1.0 -> 10.0 (1.0 = smooth fade, 10 = no fade)
vec3 _LightDirection = -sunDir;
vec3 _LightColor = vec3(1.0f);
float _LightScalar = 1.f;
float _AmbientScalar = 1.0f;
//181 157 10
vec3 _CloudTopColor = vec3(1.0f);
//vec3 _CloudBaseColor = vec3(83.f / 255.f , 96.f / 255.f, 105.f / 255.f);
vec3 _CloudBaseColor = vec3(169, 198, 255)/255.0f;
//vec3 _CloudBaseColor = vec3(94, 140, 186)/255.0f;
//vec3 _CloudBaseColor = vec3(1.0f);
float _SunRayLength = sunRayLength;
float _ConeRadius = coneRadius;
float _MaxIterations = 128;
float _MaxRayDistance = maxRayDistance;
float _RayStepLength = (atmosphereThickness / (_MaxIterations * .5f));
float _SampleScalar = 1.f;

float _ErosionEdgeSize = .5f;
float _CloudDistortion = .45f;
float _CloudDistortionScale = .5f;
float _Density = 1.0f;
float _ForwardScatteringG = 0.79f;
float _BackwardScatteringG = -0.39f;
float _DarkOutlineScalar = 1.f;

float _HorizonCoverageStart = .4f;
float _HorizonCoverageEnd = .7f;
			
float _LODDistance = .313;
float _RayMinimumY = .10;

//vec3 _Random0 = normalize(vec3(random(vec3(1.0f), 1.0f)));
//vec3 _Random1 = normalize(vec3(random(vec3(1.0f), 1.3213330f)));;
//vec3 _Random2 = normalize(vec3(random(vec3(1.0f), 5.3213210f)));;
//vec3 _Random3 = normalize(vec3(random(vec3(1.0f), 4.321310f)));;
//vec3 _Random4 = normalize(vec3(random(vec3(1.0f), 3.232130f)));;
//vec3 _Random5 = normalize(vec3(random(vec3(1.0f), 2.43430f)));;

vec3 _Random0 = vec3( 0.38051305f,  0.92453449f, -0.02111345f);
vec3 _Random1 = vec3(-0.50625799f, -0.03590792f, -0.86163418f);
vec3 _Random2 = vec3(-0.32509218f, -0.94557439f,  0.01428793f);
vec3 _Random3 = vec3( 0.09026238f, -0.27376545f,  0.95755165f);
vec3 _Random4 = vec3( 0.28128598f,  0.42443639f, -0.86065785f);
vec3 _Random5 = vec3(-0.16852403f,  0.14748697f,  0.97460106f);

const vec4 STRATUS_GRADIENT = vec4(0.02f, 0.05f, 0.09f, 0.11f);
const vec4 STRATOCUMULUS_GRADIENT = vec4(0.02f, 0.2f, 0.48f, 0.625f);
const vec4 CUMULUS_GRADIENT = vec4(0.01f, 0.0625f, 0.78f, 1.0f); // these fractions would need to be altered if cumulonimbus are added to the same pass

#define FLOAT4_COVERAGE( f)	f.r
#define FLOAT4_RAIN( f)		f.g
#define FLOAT4_TYPE( f)		f.b
			

bool is_skybox(vec3 fragPos)
{
	return distance(fragPos, viewPos) > 40000;
}
//A remapping function, that maps values from one range to another, to be used when combining noises to make our clouds
float Remap(float original_value, float original_min, float original_max, float new_min, float new_max)
{
	return new_min + (((original_value - original_min) / (original_max - original_min)) * (new_max - new_min));
}
	
//Our density height function. It is called three times, as we have three gradients for the three major cloud types. It gives us a float, representing the gradient.
//This function is called within the function "GetDensityHeightGradientForPoint". In that function, we use the weather data. More on that in a bit.
//float a is the height of the ray, namely the y value.
float DensityHeightFunction(float a, vec4 gradient)
{
	return mix(gradient.x, gradient.y, smoothstep(0, 1, a)) - mix(gradient.z, gradient.w, smoothstep(0, 1, a));
}
	
float mix3(float v0, float v1, float v2, float a)
{
	return a < 0.5 ? mix(v0, v1, a * 2.0) : mix(v1, v2, (a - 0.5) * 2.0);
}

float NormalizedAtmosphereY(vec3 ray)
{
	float y = length(ray) - _EarthRadius - _StartHeight;
	return y / _AtmosphereThickness;
}

vec4 mixGradients(float cloudType)
{
	float stratus = 1.0f - saturate(cloudType * 2.0f);
	float stratocumulus = 1.0f - abs(cloudType - 0.5f) * 2.0f;
	float cumulus = saturate(cloudType - 0.5f) * 2.0f;
	return STRATUS_GRADIENT * stratus + STRATOCUMULUS_GRADIENT * stratocumulus + CUMULUS_GRADIENT * cumulus;
}

float densityHeightGradient(
	float heightFrac,
	float cloudType)
{
	vec4 cloudGradient = mixGradients(cloudType);
	return smoothstep(cloudGradient.x, cloudGradient.y, heightFrac) - smoothstep(cloudGradient.z, cloudGradient.w, heightFrac);
}


//This function is used to figure out which clouds should be drawn and so forth
//Weather data is our weather texture channels. R is the Cloud Coverage, G is our Precipitation and B is our Cloud Type
//This function samples the B channel (Cloud type) using the ray position. 
//The sampled valus is then used to to weight the three float returns we get from the three density height functions that it calls
//In other words, the weighted sum of the gradients are affected by the cloud type attribute, which is found using the current ray positon
//P is either our current ray position or current camera position!
float GetDensityHeightGradientForPoint(vec3 p, vec3 weather_data) {
	float height = NormalizedAtmosphereY(p);

	//float gradient1 = DensityHeightFunction(p.y, _Gradient1);
	//float gradient2 = DensityHeightFunction(p.y, _Gradient2);
	//float gradient3 = DensityHeightFunction(p.y, _Gradient3);

	float gradient1 = DensityHeightFunction(p.y, vec4(0, 1.0, 0, 0));
	float gradient2 = DensityHeightFunction(p.y, vec4(0, 1.0, 0, 0));
	float gradient3 = DensityHeightFunction(p.y, vec4(0, 1.0, 0, 0));

	//float density1 = p.y
	//float weightedSum  = FLOAT4_TYPE(weather_data) * FLOAT4_TYPE(weather_data);
	float weightedSum = length(vec4(FLOAT4_TYPE(weather_data), gradient3, gradient2, gradient1));// *1 - height;
	//float weightedSum = (gradient1 + gradient2 + gradient3) * FLOAT4_TYPE(weather_data);
	//float weightedSum = FLOAT4_TYPE(weather_data) < 0.5 ? mix( v0, v1, FLOAT4_TYPE(weather_data) * 2.0) : mix( v1, v2, (FLOAT4_TYPE(weather_data)-0.5) * 2.0);
	//Do the weighted sum thingy here using the three gradients floats and the b channel of weather_data.
	//float weightedSum = weightedSum;

	float a = gradient1 + 1.0f - saturate(FLOAT4_TYPE(weather_data) / 0.5f);
	float b = gradient2 + 1.0f - abs(FLOAT4_TYPE(weather_data) - 0.5f) * 2.0f;
	float c = gradient3 + saturate(FLOAT4_TYPE(weather_data) - 0.5f) * 2.0f;

	return saturate(weightedSum);
	//return densityHeightGradient(height, FLOAT4_TYPE(weather_data));
}

//This function is used to sample the weather texture based on the ray position				
vec4 SampleWeatherTexture(vec3 ray)
{
	vec2 unit = ray.xz * _CoverageScale;
	vec2 uv = unit * 0.5 + 0.5;
	uv += _CoverageOffset;
	float depth = distance( ray, _CameraPosition) / _MaxDistance;
	vec4 coverageB = vec4( 1.0, 0.0, 0.0, 0.0);
	//coverageB.b = saturate( smoothstep( _HorizonCoverageEnd, _HorizonCoverageStart, depth) * 2.0);
	float alpha = smoothstep( _HorizonCoverageStart, _HorizonCoverageEnd, depth);
	vec4 coverage = textureLod( _WeatherTexture, uv, 0.0f);

	//return coverage;

	coverageB = vec4( smoothstep( _HorizonCoverageStart, _HorizonCoverageEnd, depth),
					0.0,
					smoothstep( _HorizonCoverageEnd, _HorizonCoverageStart + (_HorizonCoverageEnd - _HorizonCoverageStart) * 0.5, depth),
					0.0);

	return mix( coverage, coverageB, alpha);
}
	
//We use this function to transtiion between different cloud shapes by mixing all over the place!
float lerp(vec4 lowFreqNoise,vec4 neglowFreqNoise, float a){
	float mixValueR =  mix(lowFreqNoise.r,neglowFreqNoise.r, smoothstep(0,1,a));
	float mixValueG =  mix(lowFreqNoise.g,neglowFreqNoise.g, smoothstep(0,1,a));
	float mixValueB =  mix(lowFreqNoise.b,neglowFreqNoise.b, smoothstep(0,1,a));
	float mixValueA =  mix(lowFreqNoise.a,neglowFreqNoise.a, smoothstep(0,1,a));
	float sum = mixValueR +mixValueG+mixValueB+mixValueA/4;
	return sum;
}

//The main cloud moddeling function, where many of the above mentioned functions come into play
float SampleCloudDensity(vec3 ray, vec4 weather_data, float csRayHeight) 
{
	//Here we  read the first 3D texture, namely the PerlinWorleyNoise. As stated in the articel, this texture consists of 1 Perlin-Worley noise & 3 Worley noise
	//In order, each of them is going to be stored in the color channels, that is Perlin-Worley in R, and GBA is Worley
	//_Base scale will increase the size of the clouds _BaseScale+_BaseOffset
	//We have to multiply by base scale as the texture we are looking into is huge simply using the ray coordinates as a lookup
	//Will result in sampling the same area of all pixels, ergo we end up with one giant cloud in the sky
	vec4 samplingPos = vec4(ray  * _BaseScale + _BaseOffset, 0);
	vec2 inCloudMinMax = vec2(_StartHeight, _EndHeight);
	vec4 low_frequency_noises = textureLod(_PerlinWorleyNoise, samplingPos.rgb, 0).rgba;

	//Here we make an FBM out of the 3 worley noises found in the GBA channels of the low_frequency_noises.
	//We will be using this FBM to add detail to the low-frequency Perlin-Worley noise (the R channel)
	float low_freq_FBM = (low_frequency_noises.g * 0.625) + (low_frequency_noises.b * 0.25) + +(low_frequency_noises.a * 0.125);

	//Here we use our previously defined remap function to basically combine our "low_freq_FBM" with "low_frequency_noises"
	//We store this in what we will call our base_cloud
	float base_cloud = Remap(low_frequency_noises.r, -low_freq_FBM*_BaseFBMScale, 1.0, 0.0, 1.0);

	//We use the GetDensityHeightGradientForPoint to figure out which clouds should be drawn
	//vec4 density_height_gradient = vec4(GetDensityHeightGradientForPoint(ray,weather_data.rgb));

	//Here we apply height function to our base_cloud, to get the correct cloud
	//base_cloud *= density_height_gradient.r;
	base_cloud *= GetDensityHeightGradientForPoint(ray,weather_data.rgb);

	//At this point, we can stop working on base_cloud, however, it is really low-detailed and stuff (basically, you are not done with it!)
	//We need to apply the cloud coverage attribute from the weather texture to ensure that we can control how much clouds cover the sky
	//The cloud coverage is stored in the weather_data's R channel
	float cloud_coverage = weather_data.r;

	//Funny enough, we use the remap function to combine the cloud coverage with our base_cloud
	float coverageModifier = cloud_coverage;
	//if(coverageModifier == 1.0)
	//	coverageModifier -= .0001;

	float base_cloud_with_coverage = Remap(base_cloud, coverageModifier, 1.0, 0.0, 1.0);

	//float base_cloud_with_coverage = base_cloud - coverageModifier;
	//result.rgba = vec4(coverageModifier);

	//We then multipy our newly mapped base_cloud with the coverage so that we get the correct coverage of different cloud types
	//An example of this, is that smaller clouds should look lighter now. Stuff like that.
	//base_cloud_with_coverage *= cloud_coverage;
	//return base_cloud_with_coverage;


	//Next, we finish off the cloud by adding realistic detail ranging from small billows to wispy distortions
	//We use the curl noise to distort the sample coordinate at the bottom of the clouds. We do this to simulate turbulence.	
	//We will then use our mix function to transition between cloud shapes
	//First, sample the curl noise and apply it to the current position
	vec2 curl_noise = textureLod(_CurlNoise, samplingPos.xy, samplingPos.w).xy;
	ray.xy += curl_noise.xy * (1.0 - smoothstep(0,1,csRayHeight));
	

	//We then build an FBM out of our high-frequency Worley noises in order to add detail to the edges of the cloud
	//First we need to sample our noise before using it to make FBM
	vec3 high_frequency_noises = textureLod(_WorleyNoise, vec3(ray*_BaseScale *_DetailScale + _DetailOffset), 0).rgb;
	float high_freq_FBM = (high_frequency_noises.r * 0.625) + (high_frequency_noises.g * 0.25) + (high_frequency_noises.b * 0.125);
	
	//The transition magic over height happens here using our predifined mix function
	float high_freq_noise_modifier = lerp(vec4(high_freq_FBM), vec4(1.0 - high_freq_FBM), saturate(csRayHeight * 10));

	//Here we remap our cloud with the high_freq_noise_modifier
	float final_cloud = Remap(base_cloud_with_coverage, high_freq_noise_modifier*_DetailFBMScale, 1.0 , 0.0 , 1.0) ;
	//result.rgb = vec3(final_cloud);
	//return the final cloud!

	return final_cloud * _SampleScalar * smoothstep(0.0, _CloudBottomFade * 1.0, csRayHeight);
}



//Ligthing magic - courtesy of our lord and saviour, K80

//Beer’s law models the attenuation of light as it passes through a material. In our case, the clouds.
float BeerTerm(float densityAtSample)
{
	return exp(-_Density * densityAtSample);
}

//Used to increase probability of light scattering forward, to create the silver lining seen in clouds
float HenyeyGreensteinPhase(float cosAngle, float g)
{
	float g2 = g * g;
	return (1.0 - g2) / pow(1.0 + g2 - 2.0 * g * cosAngle, 1.5);
}

//In-Scattering Probability Function (Powdered Sugar Effect)
float PowderTerm(float densityAtSample, float cosTheta)
{
	//float powder = 1.0 - exp(-_Density * densityAtSample * 2.0);
	//float beers = 0.5;//exp(densityAtSample);
		
	//float sunlight = 2.0 * powder * beers;
		
	//return sunlight;

	float powder = 1.0 - exp( -_Density * densityAtSample * 2.0);
	powder = saturate( powder * _DarkOutlineScalar * 2.0);
	return mix( 1.0, powder, smoothstep( 0.5, -0.5, cosTheta));
}

//Were all the magic happens. This is ommited from the book. Genious. Again, K80 to the rescue
vec3 SampleLight(vec3 origin, float originDensity, float pixelAlpha, vec3 cosAngle, vec2 debugUV, float rayDistance, vec3 RandomUnitSphere[6])
{
	const float iterations = 5.0;

	vec3 rayStep = -_LightDirection * (_SunRayLength / iterations);
	vec3 ray = origin + rayStep;

	float atmosphereY = 0.0;

	float lod = step(0.3, originDensity) * 3.0;
	lod = 0.0;

	float value = 0.0;

	vec4 coverage;

	vec3 randomOffset = vec3(0.0, 0.0, 0.0);
	float coneRadius = 0.0;
	const float coneStep = _ConeRadius / iterations;
	float energy = 0.0;

	float thickness = 0.0;

	for (float i = 0.0; i<iterations; i++)
	{
		randomOffset = RandomUnitSphere[int(i)] * coneRadius;
		ray += rayStep;
		atmosphereY = NormalizedAtmosphereY(ray);

		coverage = SampleWeatherTexture(ray + randomOffset);
		value = SampleCloudDensity(ray + randomOffset, coverage, atmosphereY);
		value *= float(atmosphereY <= 1.0);

		thickness += value;

		coneRadius += coneStep;
	}

	float far = 8.0;
	ray += rayStep * far;
	atmosphereY = NormalizedAtmosphereY(ray);
	coverage = SampleWeatherTexture(ray);
	value = SampleCloudDensity(ray, coverage, atmosphereY);
	value *= float(atmosphereY <= 1.0);
	thickness += value;


	float forwardP = HenyeyGreensteinPhase(cosAngle.r, _ForwardScatteringG);
	float backwardsP = HenyeyGreensteinPhase(cosAngle.r, _BackwardScatteringG);
	float P = (forwardP + backwardsP) * .5f;

	return _LightColor * BeerTerm(thickness) * PowderTerm(originDensity, cosAngle.r) * P;
}

//Function used to sample the ambient light - which we sort of fake by using two color variables (representing the color of our cloud) over height (our atmosphere)
vec3 SampleAmbientLight(float atmosphereY, float depth)
{
	return mix(_CloudBaseColor, _CloudTopColor, atmosphereY);
}

vec3 FilmicTonemap(vec3 x)
{
	const float A = 0.15;
	const float B = 0.50;
	const float C = 0.10;
	const float D = 0.20;
	const float E = 0.02;
	const float F = 0.30;

	return ((x*(vec3(A)*x + vec3(C*B)) + vec3(D*E)) / (x*(vec3(A)*x + vec3(B)) + vec3(D*F))) - vec3(E / F);
}

//Fragment shader
void main()
{
	vec4 color = vec4( 0.0, 0.0, 0.0, 0.0);
	result = vec4(0.0f);

	vec3 fragPos = texture(gPosition, UV).rgb;
	if(!is_skybox(fragPos)) return;

	vec3 rayDirection = normalize( fragPos - viewPos);

	vec2 uv = (UV - 0.5) * _FieldOfView;
	uv.x *= _AspectRatio;

	if( rayDirection.y > _RayMinimumY)
	{
		// So now we have a position, and a ray defined for our current fragment, that matches the field of view and aspect ratio of the camera. 
		// We can now start iterating and creating our clouds. 
		// We will not be ray-marching twoards any distance field at this point in time.
		// pos is our original position, and p is our current position which we are going to be using later on.
		// For each iteration, we read from our SampleCloudDensity function the density of our current position, and add it to this density variable.
		float transmittance = 1.0;
		//result.a = 1.0f;
		//result.r = 1.0f;

		vec3 ray = InternalRaySphereIntersect(_EarthRadius + _StartHeight, _CameraPosition, rayDirection);
		vec3 rayStep = rayDirection * _RayStepLength;

		float atmosphereY = 0.0;
		float rayStepScalar = 1.0;

		float cosAngle = dot(rayDirection, -_LightDirection);

		float zeroThreshold = 4.0;
		float zeroAccumulator = 0.0;

		const vec3 RandomUnitSphere[6] = { _Random0, _Random1, _Random2, _Random3, _Random4, _Random5 }; ///

		float density = 1.0f;

		for (float i = 0; i < _MaxIterations; i++)
		{
			//vec2 uv = i.uv;
			if(color.a >= 1){
				break;
			}			
			// f gives a number between 0 and 1.
			// We use that to fade our clouds in and out depending on how far and close from our camera we are.
			float f = i / _MaxIterations;
			// And here we do just that:
			float alpha = smoothstep(0, _MaxIterations * 0.2, i) * (1 - f) * (1 - f);
			// Note that smoothstep here doesn't do the same as Mathf.SmoothStep() in Unity C# - which is frustrating btw. Get a grip Unity!
			// Smoothstep in shader languages interpolates between two values, given t, and returns a value between 0 and 1. 

			// At each iteration, we sample the density and add it to the density variable
			vec4 coverage = SampleWeatherTexture(ray);
			density = SampleCloudDensity(ray, coverage, atmosphereY);
			vec4 particle = vec4(density);
			
			if(density > 0.0)
			{
				//result.rgb = vec3(density) + vec3(1000.f);
				zeroAccumulator = 0;
				//Optimization code we can look at that later
				if( rayStepScalar > 1.0)
				{
					ray -= rayStep * rayStepScalar;
					i -= rayStepScalar;

					float atmosphereY = NormalizedAtmosphereY( ray);
					coverage = SampleWeatherTexture(ray);
					density = SampleCloudDensity( ray, coverage, atmosphereY);
					particle = vec4(density);
				}

				float T = 1.0 - particle.a;
				transmittance *= T;


				float dummy = 0;
				vec3 ambientLight = SampleAmbientLight(atmosphereY, dummy);
				vec3 sunLight = SampleLight(ray, particle.a, color.a, vec3(cosAngle), uv, dummy, RandomUnitSphere);

				sunLight *= _LightScalar;
				ambientLight *= _AmbientScalar;
					
				particle.a = 1.0 - T; //* transmittance;
				particle.rgb = vec3(sunLight + ambientLight);
				particle.rgb*= particle.a;
					

				//We multiply the negative alpha with the particle for god knows why
				//color.rgb
				color = (1.0 - color.a) * particle + color;
				// And then we move one step further away from the camera.
			}

			zeroAccumulator += float( density <= 0.0);
			rayStepScalar = 1.0 + step( zeroThreshold, zeroAccumulator) * 0.0;
			i += rayStepScalar;
			ray += rayStep* rayStepScalar;
			atmosphereY = NormalizedAtmosphereY( ray);
		}
		//color*= alpha;
		float fade = smoothstep(_RayMinimumY,
			_RayMinimumY + (1.0 - _RayMinimumY) * _HorizonFadeScalar,
			rayDirection.y);
		color *= _HorizonFadeStartAlpha + fade * _OneMinusHorizonFadeStartAlpha;
	}
	// If you reach this point, allelujah!

	const float Exposure = 1.5f;
	const float ExposureBias = 2.0;

	const vec3 W = vec3(3.9);
	color.rgb *= Exposure;
	vec3 curr = FilmicTonemap(ExposureBias * color.rgb);
	vec3 whiteScale = vec3(1.0f) / FilmicTonemap(W);

	result.rgb = curr * whiteScale;
	result.a = color.a;
	//result = vec4(1.0f, 0,0,1);

	//result = color;
}

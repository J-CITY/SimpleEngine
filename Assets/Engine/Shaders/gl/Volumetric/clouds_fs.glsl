#version 460 core

#define PI 3.14159265359

float saturatedDot( in vec3 a, in vec3 b )
{
	return max( dot( a, b ), 0.0 );   
}

vec3 YxyToXYZ( in vec3 Yxy )
{
	float Y = Yxy.r;
	float x = Yxy.g;
	float y = Yxy.b;

	float X = x * ( Y / y );
	float Z = ( 1.0 - x - y ) * ( Y / y );

	return vec3(X,Y,Z);
}

vec3 XYZToRGB( in vec3 XYZ )
{
	// CIE/E
	mat3 M = mat3
	(
		 2.3706743, -0.9000405, -0.4706338,
		-0.5138850,  1.4253036,  0.0885814,
 		 0.0052982, -0.0146949,  1.0093968
	);

	return XYZ * M;
}


vec3 YxyToRGB( in vec3 Yxy )
{
	vec3 XYZ = YxyToXYZ( Yxy );
	vec3 RGB = XYZToRGB( XYZ );
	return RGB;
}

void calculatePerezDistribution( in float t, out vec3 A, out vec3 B, out vec3 C, out vec3 D, out vec3 E )
{
	A = vec3(  0.1787 * t - 1.4630, -0.0193 * t - 0.2592, -0.0167 * t - 0.2608 );
	B = vec3( -0.3554 * t + 0.4275, -0.0665 * t + 0.0008, -0.0950 * t + 0.0092 );
	C = vec3( -0.0227 * t + 5.3251, -0.0004 * t + 0.2125, -0.0079 * t + 0.2102 );
	D = vec3(  0.1206 * t - 2.5771, -0.0641 * t - 0.8989, -0.0441 * t - 1.6537 );
	E = vec3( -0.0670 * t + 0.3703, -0.0033 * t + 0.0452, -0.0109 * t + 0.0529 );
}

vec3 calculateZenithLuminanceYxy( in float t, in float thetaS )
{
	float chi  	 	= ( 4.0 / 9.0 - t / 120.0 ) * ( PI - 2.0 * thetaS );
	float Yz   	 	= ( 4.0453 * t - 4.9710 ) * tan( chi ) - 0.2155 * t + 2.4192;

	float theta2 	= thetaS * thetaS;
    float theta3 	= theta2 * thetaS;
    float T 	 	= t;
    float T2 	 	= t * t;

	float xz =
      ( 0.00165 * theta3 - 0.00375 * theta2 + 0.00209 * thetaS + 0.0)     * T2 +
      (-0.02903 * theta3 + 0.06377 * theta2 - 0.03202 * thetaS + 0.00394) * T +
      ( 0.11693 * theta3 - 0.21196 * theta2 + 0.06052 * thetaS + 0.25886);

    float yz =
      ( 0.00275 * theta3 - 0.00610 * theta2 + 0.00317 * thetaS + 0.0)     * T2 +
      (-0.04214 * theta3 + 0.08970 * theta2 - 0.04153 * thetaS + 0.00516) * T +
      ( 0.15346 * theta3 - 0.26756 * theta2 + 0.06670 * thetaS + 0.26688);

	return vec3( Yz, xz, yz );
}

vec3 calculatePerezLuminanceYxy( in float theta, in float gamma, in vec3 A, in vec3 B, in vec3 C, in vec3 D, in vec3 E )
{
	return ( 1.0 + A * exp( B / cos( theta ) ) ) * ( 1.0 + C * exp( D * gamma ) + E * cos( gamma ) * cos( gamma ) );
}

vec3 calculate_sky_luminance_rgb( in vec3 s, in vec3 e, in float t )
{
	vec3 A, B, C, D, E;
	calculatePerezDistribution( t, A, B, C, D, E );

	float thetaS = acos( saturatedDot( s, vec3(0,1,0) ) );
	float thetaE = acos( saturatedDot( e, vec3(0,1,0) ) );
	float gammaE = acos( saturatedDot( s, e )		   );

	vec3 Yz = calculateZenithLuminanceYxy( t, thetaS );

	vec3 fThetaGamma = calculatePerezLuminanceYxy( thetaE, gammaE, A, B, C, D, E );
	vec3 fZeroThetaS = calculatePerezLuminanceYxy( 0.0,    thetaS, A, B, C, D, E );

	vec3 Yp = Yz * ( fThetaGamma / fZeroThetaS );

	return YxyToRGB( Yp );
}

//#include <atmosphere.glsl>

#define NUM_CONE_SAMPLES 6

// ------------------------------------------------------------------
// OUTPUT VARIABLES  ------------------------------------------------
// ------------------------------------------------------------------

out vec4 FS_OUT_Color;

// ------------------------------------------------------------------
// INPUT VARIABLES  -------------------------------------------------
// ------------------------------------------------------------------

in vec2 FS_IN_TexCoord;

// ------------------------------------------------------------------
// STRUCTURES -------------------------------------------------------
// ------------------------------------------------------------------

struct Ray
{
    vec3 origin;
    vec3 direction;
};

// ------------------------------------------------------------------
// UNIFORMS ---------------------------------------------------------
// ------------------------------------------------------------------

//layout(std140, binding = 0) uniform GlobalUniforms
//{
//    mat4 view_proj;
//    mat4 inv_view_proj;
//    vec4 cam_pos;
//};

layout (std140, binding = 0) uniform EngineUBO {
    mat4    View;
    mat4    Projection;
    vec3    ViewPos;
    float   Time;
    vec2    ViewportSize;
    float    FPS;
    int      FrameCount;
} engine_UBO;

struct PushModel {
    mat4 model;
};
uniform PushModel engine_Model;

uniform sampler3D engine_s_ShapeNoise;
uniform sampler3D engine_s_DetailNoise;
uniform sampler2D s_BlueNoise;
uniform sampler2D s_CurlNoise;

uniform vec3  	  u_PlanetCenter;
uniform float 	  u_PlanetRadius;
uniform float 	  u_CloudMinHeight;
uniform float 	  u_CloudMaxHeight;
uniform float 	  u_ShapeNoiseScale;
uniform float 	  u_DetailNoiseScale;
uniform float 	  u_DetailNoiseModifier;
uniform float 	  u_TurbulenceNoiseScale;
uniform float 	  u_TurbulenceAmount;
uniform float 	  u_CloudCoverage;
//uniform vec3 	  u_WindDirection;
uniform float 	  u_WindAngle;
uniform float	  u_WindSpeed;
uniform float	  u_WindShearOffset;
//uniform float     u_Time;
uniform float	  u_MaxNumSteps;
uniform float 	  u_LightStepLength;
uniform float 	  u_LightConeRadius;
uniform vec3      engine_u_SunDir;
uniform vec3      u_SunColor;
uniform vec3      u_CloudBaseColor;
uniform vec3      u_CloudTopColor;
uniform float 	  u_Precipitation;
uniform float 	  u_AmbientLightFactor;
uniform float 	  u_SunLightFactor;
uniform float 	  u_HenyeyGreensteinGForward;
uniform float 	  u_HenyeyGreensteinGBackward;


mat4 view_proj;
mat4 inv_view_proj;
vec3 windDirection;

// ------------------------------------------------------------------
// FUNCTIONS --------------------------------------------------------
// ------------------------------------------------------------------

Ray generate_ray()
{
    vec2 tex_coord_neg_to_pos = FS_IN_TexCoord.xy * 2.0f - 1.0f;
    vec4 target = inv_view_proj * vec4(tex_coord_neg_to_pos, 0.0f, 1.0f);
    target /= target.w;

    Ray ray;

    ray.origin 	  = engine_UBO.ViewPos.xyz;
    ray.direction = normalize(target.xyz - ray.origin);

    return ray;
}

// ------------------------------------------------------------------

vec3 ray_sphere_intersection(in Ray ray, in vec3 sphere_center, in float sphere_radius)
{
    vec3 l = ray.origin - sphere_center;
	float a = 1.0;
	float b = 2.0 * dot(ray.direction, l);
	float c = dot(l, l) - pow(sphere_radius, 2);
	float D = pow(b, 2) - 4.0 * a * c;
	
    if (D < 0.0)
		return ray.origin;
	else if (abs(D) - 0.00005 <= 0.0)
		return ray.origin + ray.direction * (-0.5 * b / a);
	else
	{
		float q = 0.0;
		if (b > 0.0) 
            q = -0.5 * (b + sqrt(D));
		else 
            q = -0.5 * (b - sqrt(D));
		
		float h1 = q / a;
		float h2 = c / q;
		vec2 t = vec2(min(h1, h2), max(h1, h2));

		if (t.x < 0.0) 
        {
			t.x = t.y;
			
            if (t.x < 0.0)
				return ray.origin;
		}
        
		return ray.origin + t.x * ray.direction;
	}
}

// ------------------------------------------------------------------

float blue_noise()
{
	ivec2 size = textureSize(s_BlueNoise, 0);

	vec2 interleaved_pos = (mod(floor(gl_FragCoord.xy), float(size.x)));
	vec2 tex_coord 	     = interleaved_pos / float(size.x) + vec2(0.5f / float(size.x), 0.5f / float(size.x));
	
	return texture(s_BlueNoise, tex_coord).r * 2.0f - 1.0f;
}

// ------------------------------------------------------------------

float remap(float original_value, float original_min, float original_max, float new_min, float new_max)
{
	return new_min + (((original_value - original_min) / (original_max - original_min)) * (new_max - new_min));
}

// ------------------------------------------------------------------

// returns height fraction [0, 1] for point in cloud
float height_fraction_for_point(vec3 _position)
{
	return clamp((distance(_position,  u_PlanetCenter) - (u_PlanetRadius + u_CloudMinHeight)) / (u_CloudMaxHeight - u_CloudMinHeight), 0.0f, 1.0f);
}

// ------------------------------------------------------------------

float density_height_gradient_for_point(vec3 _position, float _height_fraction)
{
	return 1.0f;
}

// ------------------------------------------------------------------

float sample_cloud_density(vec3 _position, float _height_fraction, float _lod, bool _use_detail)
{
    // Shear cloud top along wind direction.
    vec3 position = _position + windDirection * u_WindShearOffset * _height_fraction; 

    // Animate clouds in wind direction and add a small upward bias to the wind direction.
    position += (windDirection + vec3(0.0f, 0.1f, 0.0f)) * u_WindSpeed * engine_UBO.Time;

    // Read the low-frequency Perlin-Worley and Worley noises.
    vec4 low_frequency_noises = textureLod(engine_s_ShapeNoise, position * u_ShapeNoiseScale, _lod);

    // Build an FBM out of the low-frequency Worley noises to add detail to the low-frequeny Perlin-Worley noise.
    float low_freq_fbm = (low_frequency_noises.g * 0.625f) + (low_frequency_noises.b * 0.25f) + (low_frequency_noises.a * 0.125f);

    // Define the base cloud shape by dilating it with the low-frequency FBM made of Worley noise.
    float base_cloud = remap(low_frequency_noises.r, (1.0f - low_freq_fbm), 1.0f, 0.0f, 1.0f);

    // Get the density-height gradient using the density height function.
    float density_height_gradient = density_height_gradient_for_point(position, _height_fraction);

    // Apply the height function to the base cloud shape.
    base_cloud *= density_height_gradient;

    // Fetch cloud coverage value.
    float cloud_coverage = u_CloudCoverage;

    // Remap to apply the cloud coverage attribute.
    float base_cloud_with_coverage = remap(base_cloud, cloud_coverage, 1.0f, 0.0f, 1.0f);

    // Multiply result by the cloud coverage attribute so that smaller clouds are lighter and more aesthetically pleasing.
    base_cloud_with_coverage *= cloud_coverage;

    // Exit out if base cloud density is zero.
    if (base_cloud_with_coverage <= 0.0f)
        return 0.0f;

    float final_cloud = base_cloud_with_coverage;

    if (_use_detail)
    {
        // Sample curl noise texture.
        vec2 curl_noise = textureLod(s_CurlNoise, position.xz * u_TurbulenceNoiseScale, 0.0f).rg;
        
        // Add some turbulence to bottom of clouds.
        position.xy += curl_noise * (1.0f - _height_fraction) * u_TurbulenceAmount;

        // Sample high-frequency noises.
        vec3 high_frequency_noises = textureLod(engine_s_DetailNoise, position * u_DetailNoiseScale, _lod).rgb;

        // Build high-frequency Worley noise FBM.
        float high_freq_fbm = (high_frequency_noises.r * 0.625f) + (high_frequency_noises.g * 0.25f) + (high_frequency_noises.b * 0.125f);

        // Transition from wispy shapes to billowy shapes over height.
        float high_freq_noise_modifier = mix(1.0f - high_freq_fbm, high_freq_fbm, clamp(_height_fraction * 10.0f, 0.0f, 1.0f));

        // Erode the base cloud shape with the distorted high-frequency Worley noise.
        final_cloud = remap(base_cloud_with_coverage, high_freq_noise_modifier * u_DetailNoiseModifier, 1.0f, 0.0f, 1.0f);
    }

    return clamp(final_cloud, 0.0f, 1.0f);
}

// ------------------------------------------------------------------

float sample_cloud_density_along_cone(vec3 _position, vec3 _light_dir)
{
	const vec3 noise_kernel[6] = 
	{
		{ -0.6, -0.8, -0.2 },
		{ 1.0, -0.3, 0.0 },
		{ -0.7, 0.0, 0.7 },
		{ -0.2, 0.6, -0.8 },
		{ 0.4, 0.3, 0.9 },
		{ -0.2, 0.6, -0.8 }
	};

	float density_along_cone = 0.0f;

	for (int i = 0; i < NUM_CONE_SAMPLES; i++) 
	{
        // March ray forward along light direction.
		_position += _light_dir * u_LightStepLength;

        // Compute offset within the cone. 
		vec3 random_offset = noise_kernel[i] * u_LightStepLength * u_LightConeRadius * (float(i + 1));

        // Add offset to position.
		vec3 p = _position + random_offset;
		
        // Compute height fraction for the current position.
		float height_fraction = height_fraction_for_point(p);

		// Skipping detail noise based on accumulated density causes some banding artefacts 
		// so only use detail noise for the first two samples.   
        bool use_detail_noise = i < 2;
            
        // Sample the cloud density at this point within the cone.
        density_along_cone += sample_cloud_density(p, height_fraction, float(i) * 0.5f, use_detail_noise);
	}

    // Get one more sample further away to account for shadows from distant clouds.
	_position += 32.0f * u_LightStepLength * _light_dir; 

    // Compute height fraction for the distant position.
	float height_fraction = height_fraction_for_point(_position);

    // Sample the cloud density for the distant position.
	density_along_cone += sample_cloud_density(_position, height_fraction, 2.0f, false) * 3.0f;
	
	return density_along_cone;
}

// ------------------------------------------------------------------

float beer_lambert_law(float _density)
{
    return exp(-_density * u_Precipitation);
}

// ------------------------------------------------------------------

float beer_law(float density)
{
	float d = -density * u_Precipitation;
	return max(exp(d), exp(d * 0.5f) * 0.7f);
}

// ------------------------------------------------------------------

float henyey_greenstein_phase(float cos_angle, float g)
{
	float g2 = g * g;
	return ((1.0f - g2) / pow(1.0f + g2 - 2.0f * g * cos_angle, 1.5f)) / 4.0f * 3.1415f;
}

// ------------------------------------------------------------------

float powder_effect(float _density, float _cos_angle)
{
	float powder = 1.0f - exp(-_density * 2.0f);
	return mix(1.0f, powder, clamp((-_cos_angle * 0.5f) + 0.5f, 0.0f, 1.0f));
}

// ------------------------------------------------------------------

float calculate_light_energy(float _density, float _cos_angle, float _powder_density) 
{ 
	float beer_powder = 2.0f * beer_law(_density) * powder_effect(_powder_density, _cos_angle);
	float HG = max(henyey_greenstein_phase(_cos_angle, u_HenyeyGreensteinGForward), henyey_greenstein_phase(_cos_angle, u_HenyeyGreensteinGBackward)) * 0.07f + 0.8f;
	return beer_powder * HG;
}

// ------------------------------------------------------------------

vec4 ray_march(vec3 _ray_origin, vec3 _ray_direction, float _cos_angle, float _step_size, float _num_steps)
{
	vec3  position            = _ray_origin;
	float step_increment      = 1.0f;
    float accum_transmittance = 1.0f;
    vec3  accum_scattering    = vec3(0.0f);
    float alpha               = 0.0f;

	vec3 sun_color = u_SunColor;

	for (float i = 0.0f; i < _num_steps; i+= step_increment)
	{
		float height_fraction    = height_fraction_for_point(position);
		float density            = sample_cloud_density(position, height_fraction, 0.0f, true);
        float step_transmittance = beer_lambert_law(density * _step_size);

        accum_transmittance *= step_transmittance;

		if (density > 0.0f)
		{
            alpha += (1.0f - step_transmittance) * (1.0f - alpha);
            
            float cone_density = sample_cloud_density_along_cone(position, engine_u_SunDir);

            vec3 in_scattered_light = calculate_light_energy(cone_density * _step_size, _cos_angle, density * _step_size) * sun_color * u_SunLightFactor * alpha;
            vec3 ambient_light      = mix(u_CloudBaseColor, u_CloudTopColor, height_fraction) * u_AmbientLightFactor;

            accum_scattering += (ambient_light + in_scattered_light) * accum_transmittance * density;
		}

		position += _ray_direction * _step_size * step_increment;
	}

	return vec4(accum_scattering, alpha);
}

// ------------------------------------------------------------------
// MAIN -------------------------------------------------------------
// ------------------------------------------------------------------

void main()
{
	
	view_proj = engine_UBO.Projection * engine_UBO.View;
	inv_view_proj = inverse(view_proj);
	windDirection  = normalize(vec3(cos(u_WindAngle), sin(u_WindAngle), 0.0f));

	// Generate a camera ray to the current fragment.
	Ray ray = generate_ray();

	// Figure out where our ray will intersect the sphere that represents the beginning of the cloud layer. 
	vec3 ray_start = ray_sphere_intersection(ray, u_PlanetCenter, u_PlanetRadius + u_CloudMinHeight);

	// Similarly, figure out where the ray intersects the end of the cloud layer.
	vec3 ray_end   = ray_sphere_intersection(ray, u_PlanetCenter, u_PlanetRadius + u_CloudMaxHeight);

	// Get a random number that we'll use to jitter our ray.
	const float rng = blue_noise();
	
	// The maximum number of ray march steps to use.
	const float max_steps = u_MaxNumSteps;
	
	// The minimum number of ray march steps to use with an added offset to prevent banding.
	const float min_steps = (u_MaxNumSteps * 0.5f) + (rng * 2.0f);

	// The number of ray march steps is determined depending on how steep the viewing angle is.
	float num_steps = mix(max_steps, min_steps, ray.direction.y);

	// Using the number of steps we can determine the step size of our ray march.
	float step_size = length(ray_start - ray_end) / num_steps;

	// Jitter the ray to prevent banding.
	ray_start += step_size * ray.direction * rng;

	float cos_angle = dot(ray.direction, engine_u_SunDir);
	vec4  clouds    = ray_march(ray_start, ray.direction, cos_angle, step_size, num_steps);
	vec4  sky       = vec4(calculate_sky_luminance_rgb(engine_u_SunDir, ray.direction, 2.0f) * 0.05f, 1.0f);

	FS_OUT_Color = vec4(clouds.rgb + (1.0f - clouds.a) * sky.rgb, 1);
	//FS_OUT_Color = vec4(1,1,1, 1);
}

// ------------------------------------------------------------------
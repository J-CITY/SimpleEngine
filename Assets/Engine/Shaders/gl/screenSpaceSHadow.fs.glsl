

const int  g_sss_max_steps                      = 12;    // Max ray steps, affects quality and performance.
const float g_sss_ray_max_distance              = 0.05; // Max shadow length, longer shadows are less accurate.
const float g_sss_thickness                     = 0.05; // Depth testing thickness.
const float g_sss_max_delta_from_original_depth = 0.005; // The maximum allowed depth deviation from the original pixel (a big deviation decreased the probabilty that the pixel is the occluder).
const float g_sss_step_length                   = g_sss_ray_max_distance / (float)g_sss_max_steps;

const float RPC_9               = 0.11111111111;
const float RPC_16              = 0.0625;

float get_noise_interleaved_gradient(vec2 screen_pos) {
    // Temporal factor
    float taaOn      = true;//(float)is_taa_enabled();
    float frameCount = (float)g_frame;
    float frameStep  = taaOn * float(frameCount % 16) * RPC_16;
    screen_pos.x     += frameStep * 4.7526;
    screen_pos.y     += frameStep * 3.1914;

    vec3 magic = vec3(0.06711056f, 0.00583715f, 52.9829189f);
    return frac(magic.z * frac(dot(screen_pos, magic.xy)));
}

vec3 world_to_view(vec3 x, bool is_position) {
    return (engine_UBO.View * vec4(x, (float)is_position)).xyz;
}

// Derived from the interleaved gradient function from Jimenez 2014 http://goo.gl/eomGso
float get_noise_interleaved_gradient(float2 screen_pos) {
    // Temporal factor
    float taaOn      = true;//(float)is_taa_enabled();
    float frameCount = (float)g_frame;
    float frameStep  = taaOn * float(frameCount % 16) * RPC_16;
    screen_pos.x     += frameStep * 4.7526;
    screen_pos.y     += frameStep * 3.1914;

    float3 magic = float3(0.06711056f, 0.00583715f, 52.9829189f);
    return frac(magic.z * frac(dot(screen_pos, magic.xy)));
}

vec2 view_to_uv(vec3 x, bool is_position) {
    vec4 uv = engine_UBO.Projection * vec4(x, (float)is_position);
    return (uv.xy / uv.w) * vec2(0.5f, -0.5f) + 0.5f;
}

bool is_saturated(float value)  { return value == saturate(value); }
bool is_saturated(vec2 value) { return is_saturated(value.x) && is_saturated(value.y); }
bool is_saturated(vec3 value) { return is_saturated(value.x) && is_saturated(value.y) && is_saturated(value.z); }
bool is_saturated(vec4 value) { return is_saturated(value.x) && is_saturated(value.y) && is_saturated(value.z) && is_saturated(value.w); }

float get_depth(vec2 uv) {
    // effects like screen space shadows, can get artefacts if a point sampler is used
    return tex_depth.SampleLevel(sampler_bilinear_clamp, uv, 0).r;
}

float get_linear_depth(float z, float near, float far) {
    float z_b = z;
    float z_n = 2.0f * z_b - 1.0f;
    return 2.0f * far * near / (near + far - z_n * (near - far));
}

float get_linear_depth(float z) {
    return get_linear_depth(z, g_camera_near, g_camera_far);
}

float ScreenSpaceShadows(vec3 position, vec2 uv, Light light) {
    // Compute ray position and direction (in view-space)
    vec3 ray_pos = world_to_view(position, true);
    vec3 ray_dir = world_to_view(-light.to_pixel, false);

    // Compute ray step
    vec3 ray_step = ray_dir * g_sss_step_length;

    // Offset starting position with temporal interleaved gradient noise
    float offset = get_noise_interleaved_gradient(uv * g_resolution_rt);
    ray_pos      += ray_step * offset;
    
    // Save the original depth
    float depth_original = ray_pos.z;

    // Ray march towards the light
    float occlusion = 0.0;
    vec2 ray_uv   = 0.0f;
    for (uint i = 0; i < g_sss_max_steps; i++) {
        // Step the ray
        ray_pos += ray_step;
        ray_uv  = view_to_uv(ray_pos, true);

        // Ensure the UV coordinates are inside the screen
        if (!is_saturated(ray_uv)) {
            return 1.0f;
        }
        // Compute the difference between the ray's and the camera's depth
        float depth_z       = get_linear_depth(position.z);
        float depth_delta   = ray_pos.z - depth_z;

        bool can_the_camera_see_the_ray     = (depth_delta > 0.0f) && (depth_delta < g_sss_thickness);
        bool occluded_by_the_original_pixel = abs(ray_pos.z - depth_original) < g_sss_max_delta_from_original_depth;
        if (can_the_camera_see_the_ray && occluded_by_the_original_pixel) {
            // Mark as occluded
            occlusion = 1.0f;

            // Fade out as we approach the edges of the screen
            occlusion *= screen_fade(ray_uv);
            break;
        }
    }

    // Convert to visibility
    return 1.0f - occlusion;
}
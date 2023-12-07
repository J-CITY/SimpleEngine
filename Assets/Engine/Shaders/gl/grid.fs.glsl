#version 460 core
#include "../lib/engineUBO.glsl"

#ifdef GL_FRAGMENT_PRECISION_HIGH
  precision highp float;
#else
  precision mediump float;
#endif
layout(location = 0) out vec4 o_color;

in vec3 near;
in vec3 far;

float u_near = 0.1;
float u_far = 1000.0;


vec4 grid(vec3 point, float scale, bool is_axis) {
    vec2 coord = point.xz * scale;
    vec2 dd    = fwidth(coord);
    vec2 uv    = fract(coord - 0.5) - 0.5;
    vec2 grid  = abs(uv) / dd;  // TODO: figure this out, adjust the line thickness
    float line = min(grid.x, grid.y);
    float min_z = min(dd.y, 1.0);
    float min_x = min(dd.x, 1.0);
    vec4 col    = vec4(0.3);
    col.a       = 1.0 - min(line, 1.0);


    if (-1.0 * min_x < point.x && point.x < 0.1 * min_x && is_axis)
        col.rgb = vec3(0.427, 0.792, 0.909);
    if (-1.0 * min_z < point.z && point.z < 0.1 * min_z && is_axis)
        col.rgb = vec3(0.984, 0.380, 0.490);
    return col;
}

float compute_depth(vec3 point) {
    vec4 clip_space = engine_UBO.Projection * engine_UBO.View * vec4(point, 1.0);
    float clip_space_depth = clip_space.z / clip_space.w;
    float far  = gl_DepthRange.far;
    float near = gl_DepthRange.near;
    float depth = (((far - near) * clip_space_depth) + near + far) / 2.0;
    return depth;
}

float compute_fade(vec3 point) {
    vec4 clip_space = engine_UBO.Projection * engine_UBO.View * vec4(point, 1.0);
    float clip_space_depth = (clip_space.z / clip_space.w) * 2.0 - 1.0;
    float near = u_near;
    float far  = u_far;
    float linear_depth = (2.0 * near * far) / (far + near - clip_space_depth * (far - near));
    return linear_depth / far;
}

void main() {
    float t = -near.y / (far.y - near.y);
    vec3  R =  near + t * (far - near);
    float is_on = float(t > 0);

    //float fade = max(0, 1.0 - compute_fade(R));
    float fade = smoothstep(0.04, 0.0, compute_fade(R));
    o_color = grid(R, 1, true);
    //o_color += grid(R, 10, false) * 0.25;
    o_color *= fade;

    o_color *= is_on;
    gl_FragDepth = compute_depth(R);
}

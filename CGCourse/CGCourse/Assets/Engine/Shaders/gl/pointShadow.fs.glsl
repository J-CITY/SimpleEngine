#version 460 core

layout (std140, binding = 1) uniform EnginePointShadowUBO {
    vec3 lightPos;
    float far_plane;
    mat4 shadowMatrices[6];
} engine_PointShadowUBO;

in vec4 FragPos;

void main() {
    float lightDistance = length(FragPos.xyz - engine_PointShadowUBO.lightPos);
    lightDistance = lightDistance / engine_PointShadowUBO.far_plane;
    gl_FragDepth = lightDistance;
}

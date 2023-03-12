#version 460 core

out vec4 FragColor;

in vec2 TexCoords;

//layout (std140) uniform EngineUBO
//{
//    mat4    ubo_Model;
//    mat4    ubo_View;
//    mat4    ubo_Projection;
//    vec3    ubo_ViewPos;
//    float   ubo_Time;
//    vec2    ubo_ViewportSize;
//};

uniform sampler2D u_Scene;
uniform float u_Radius;
uniform float u_Intensity;

vec3 applyVignette(vec3 color, vec2 texcoord, float radius, float intensity) {
    texcoord *= 1.0f - texcoord.yx;
    float vig = texcoord.x * texcoord.y * intensity;
    vig = pow(vig, radius);
    return color * min(vig, 1.0f);
}

void main() {
    vec3 color = texture(u_Scene, TexCoords).rgb;
    //color = applyVignette(color, TexCoords, u_Radius, u_Intensity);
    FragColor = vec4(color, 1.0f);
}

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
uniform vec3 u_ChannelR;
uniform vec3 u_ChannelG;
uniform vec3 u_ChannelB;

void main() {
    vec3 inputColor = texture(u_Scene, TexCoords).rgb;
    vec3 outputColor = u_ChannelR * inputColor.r + u_ChannelG * inputColor.g + u_ChannelB * inputColor.b;
    outputColor = min(vec3(1), outputColor);
    FragColor = vec4(outputColor, 1.0f);
}

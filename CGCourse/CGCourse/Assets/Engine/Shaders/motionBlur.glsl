#shader vertex
#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 TexCoords;

void main() {
    TexCoords = aTexCoords;
    gl_Position = vec4(aPos, 1.0);
}

#shader fragment
#version 460 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D u_SceneBuffer0;
uniform sampler2D u_SceneBuffer1;
uniform sampler2D u_SceneBuffer2;
uniform sampler2D u_SceneBuffer3;
uniform bool u_UseMotionBlur;

layout (std140) uniform EngineUBO
{
    mat4    ubo_Model;
    mat4    ubo_View;
    mat4    ubo_Projection;
    vec3    ubo_ViewPos;
    float   ubo_Time;
};

void main() {
    vec3 color0 = texture(u_SceneBuffer0, TexCoords).rgb;
    vec3 color1 = texture(u_SceneBuffer1, TexCoords).rgb;
    vec3 color2 = texture(u_SceneBuffer2, TexCoords).rgb;
    vec3 color3 = texture(u_SceneBuffer3, TexCoords).rgb;

    vec3 color = vec3(0.0);
    if (u_UseMotionBlur) {
        color += color3 * 0.4;
        color += color2 * 0.3;
        color += color1 * 0.2;
        color += color0 * 0.1;
    }
    else {
        color = color3;
    }
    FragColor = vec4(color, 1.0);
}

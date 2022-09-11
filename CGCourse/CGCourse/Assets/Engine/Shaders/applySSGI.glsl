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

layout (std140) uniform EngineUBO {
    mat4    ubo_Model;
    mat4    ubo_View;
    mat4    ubo_Projection;
    vec3    ubo_ViewPos;
    float   ubo_Time;
    vec2    ubo_ViewportSize;
};

uniform sampler2D inputTex;
uniform sampler2D SSGITex;
uniform sampler2D albedoTex;

void main() {
    vec3 originalColor = texture(inputTex, TexCoords, 0).rgb;
    vec3 ssgiColor = texture(SSGITex, TexCoords, 0).rgb;

    vec3 albedo = texture(albedoTex, TexCoords).rgb;
    float metallic = texture(albedoTex, TexCoords).a;
    vec3 color = mix(albedo, originalColor, metallic * 0.5);

    FragColor = vec4(originalColor + color * ssgiColor, 1.0);
}

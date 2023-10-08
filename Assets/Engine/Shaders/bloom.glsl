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

uniform sampler2D u_Scene;
uniform sampler2D u_BloomBlur;
uniform bool u_UseBloom;

void main() {
    vec3 color = texture(u_Scene, TexCoords).rgb;
    vec3 bloomColor = texture(u_BloomBlur, TexCoords).rgb;
    if(u_UseBloom) {
        color += bloomColor;
    }
    FragColor = vec4(color, 1.0);
}

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

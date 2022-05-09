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

in vec2 TexCoords;
out vec4 OutColor;

uniform sampler2D u_Scene;

void main() {
    vec3 color = texture(u_Scene, TexCoords).rgb;
    float brightness = dot(color, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 1.0) {
        OutColor = vec4(color, 1.0);
    }
    else {
        OutColor = vec4(0.0, 0.0, 0.0, 1.0);
    }
}

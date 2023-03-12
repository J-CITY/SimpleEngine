#version 460 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D image;
uniform vec4 spriteColor = vec4(1,1,1,1);

void main() {
    color = spriteColor * texture(image, TexCoords);
}

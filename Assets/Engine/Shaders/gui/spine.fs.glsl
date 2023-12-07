#version 460 core

in vec2 TexCoords;
in vec4 OutColor;

out vec4 color;

uniform sampler2D image;
//uniform vec4 spriteColor = vec4(1,1,1,1);

void main() {
    //color = spriteColor * texture(image, TexCoords);
    color = OutColor * texture2D(image, TexCoords);

    //color = vec4(1,0,0,1);
}

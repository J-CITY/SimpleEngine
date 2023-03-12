#version 440

#include "../lib/engineUBO.glsl"

in vec2 TexCoords;

out vec4 outColor;

uniform sampler2D velTex;
uniform sampler2D colorTexture;

int   size       = 2;
float separation = 1.0;

void main() {
    vec2 direction = texture(velTex, TexCoords).rg;

    if (length(direction) <= 0.0) {
        outColor = texture(colorTexture, TexCoords);
        //outColor = vec4(1,0,0,1);
        return; 
    }

    direction.xy *= separation;

    vec2 forward  = TexCoords;
    vec2 backward = TexCoords;

    float count = 1.0;
    for (int i = 0; i < size; ++i) {
        forward  += direction;
        backward -= direction;
        outColor +=texture(colorTexture, forward);
        outColor += texture(colorTexture, backward);
        count += 2.0;
    }

    outColor /= count;

    //outColor = vec4(direction,0,1);
}
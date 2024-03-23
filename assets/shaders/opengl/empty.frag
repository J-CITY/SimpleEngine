#version 120

uniform vec4 u_Color;

varying vec2 v_texCoord;

void main() {
    gl_FragColor = u_Color;
}

#version 120

uniform sampler2D inputTexture;
varying vec2 v_texCoord;

void main() {
    vec3 fragPos = texture2D(inputTexture, v_texCoord).rgb;
    gl_FragColor = vec4(fragPos, 1.0);
}

#version 120

attribute vec3 geo_Pos;
attribute vec2 geo_TexCoords;

varying vec2 v_texCoord;

void main() {
    v_texCoord = geo_TexCoords;
    gl_Position = vec4(geo_Pos, 1.0);
}

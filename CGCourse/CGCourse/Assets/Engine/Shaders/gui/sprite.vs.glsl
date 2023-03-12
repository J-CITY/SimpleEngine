#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;
out vec2 TexCoords;
 
uniform mat4 u_engine_model;
uniform mat4 u_engine_projection;
 
void main() {
    TexCoords = aTexCoords;
    //gl_Position = u_engine_projection * u_engine_model * vec4(aPos.xy, 0.0, 1.0);
    gl_Position = u_engine_projection * u_engine_model * vec4(aPos.xy, 0.0, 1.0);
}

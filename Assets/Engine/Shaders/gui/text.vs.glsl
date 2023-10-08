#version 460 core

layout (location = 0) in vec4 vertex;
out vec2 TexCoords;

uniform mat4 u_engine_model;
uniform mat4 u_engine_projection;

void main() {
    TexCoords = vertex.zw;
    gl_Position = u_engine_projection * u_engine_model * vec4(vertex.xy, 0.0, 1.0);
    //gl_Position = ubo_Projection * (ubo_View * (model * vec4(vertex.xy, 0.0, 1.0)));
}
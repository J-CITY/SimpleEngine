#shader vertex
#version 460 core

layout (std140) uniform EngineUBO {
    mat4    ubo_Model;
    mat4    ubo_View;
    mat4    ubo_Projection;
    vec3    ubo_ViewPos;
    float   ubo_Time;
    vec2    ubo_ViewportSize;
};

layout (location = 0) in vec4 vertex;
out vec2 TexCoords;

uniform mat4 u_engine_model;
uniform mat4 u_engine_projection;

void main() {
    TexCoords = vertex.zw;
    gl_Position = u_engine_projection * u_engine_model * vec4(vertex.xy, 0.0, 1.0);
    //gl_Position = ubo_Projection * (ubo_View * (model * vec4(vertex.xy, 0.0, 1.0)));
}

#shader fragment
#version 460 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D u_engine_text;
uniform vec3 textColor;

void main() {
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(u_engine_text, TexCoords).r);
    color = vec4(textColor, 1.0) * sampled;
}

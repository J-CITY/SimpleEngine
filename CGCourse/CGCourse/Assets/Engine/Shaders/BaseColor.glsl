#shader vertex
#version 460 core

layout (location = 0) in vec3 geo_Pos;
layout (location = 1) in vec2 geo_TexCoords;
layout (location = 2) in vec3 geo_Normal;
layout (location = 3) in vec3 geo_Tangent;
layout (location = 4) in vec3 geo_Bitangent;
layout (location = 5) in ivec4 boneIds;
layout (location = 6) in vec4 weights;

/* Global information sent by the engine */

layout (std140) uniform EngineUBO {
    mat4    ubo_Model;
    mat4    ubo_View;
    mat4    ubo_Projection;
    vec3    ubo_ViewPos;
    float   ubo_Time;
    vec2    ubo_ViewportSize;
};

/* Information passed to the fragment shader */
out VS_OUT {
    vec3        FragPos;
    vec2        TexCoords;
} vs_out;

void main() {
    vs_out.FragPos          = vec3(ubo_Model * vec4(geo_Pos, 1.0));
    vs_out.TexCoords        = geo_TexCoords;
    gl_Position = ubo_Projection * (ubo_View * vec4(vs_out.FragPos, 1.0));
    //gl_Position = vec4(geo_Pos, 1.0);
}

#shader fragment
#version 460 core

/* Global information sent by the engine */
layout (std140) uniform EngineUBO
{
    mat4    ubo_Model;
    mat4    ubo_View;
    mat4    ubo_Projection;
    vec3    ubo_ViewPos;
    float   ubo_Time;
    vec2    ubo_ViewportSize;
};

/* Information passed from the fragment shader */
in VS_OUT
{
    vec3        FragPos;
    vec2        TexCoords;
} fs_in;

uniform vec3 color;

out vec4 FRAGMENT_COLOR;

void main() {
    FRAGMENT_COLOR = vec4(color, 1);
}

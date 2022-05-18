#shader vertex
#version 460 core

layout (location = 0) in vec3 geo_Pos;
layout (location = 1) in vec2 geo_TexCoords;
layout (location = 2) in vec3 geo_Normal;
layout (location = 3) in vec3 geo_Tangent;
layout (location = 4) in vec3 geo_Bitangent;

layout(location = 5) in ivec4 boneIds;
layout(location = 6) in vec4 weights;

layout (std140) uniform EngineUBO
{
    mat4    ubo_Model;
    mat4    ubo_View;
    mat4    ubo_Projection;
    vec3    ubo_ViewPos;
    float   ubo_Time;
};

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;

uniform mat4 u_engine_FinalBonesMatrices[MAX_BONES];
uniform bool u_UseBone;

uniform mat4 model;

void main()
{
    vec4 totalPosition = vec4(0.0f);
    if (u_UseBone) {
        for(int i = 0; i < MAX_BONE_INFLUENCE; i++) {
            if(int(boneIds[i]) == -1) 
                continue;
            if(int(boneIds[i]) >=MAX_BONES) {
                totalPosition = vec4(geo_Pos,1.0f);
                break;
            }
            vec4 localPosition = (u_engine_FinalBonesMatrices[int(boneIds[i])]) * vec4(geo_Pos,1.0f);
            totalPosition += localPosition * weights[i];
        }
    }

    if (u_UseBone) {
        gl_Position = model * totalPosition;
    }
    //else {
        gl_Position = model * vec4(geo_Pos, 1.0);
    //}
}

#shader fragment
#version 460 core

uniform vec3 lightPos;
uniform float far_plane;

in vec4 FragPos; 

void main()
{             
    float lightDistance = length(FragPos.xyz - lightPos);
    
    // Приводим значение к диапазону [0; 1] путем деления на far_plane
    lightDistance = lightDistance / far_plane;
    
    // Записываем его в качестве измененной глубины
    gl_FragDepth = lightDistance;
}

#shader geometry
#version 460 core

layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;

uniform mat4 shadowMatrices[6];

out vec4 FragPos; 

void main() {
    for(int face = 0; face < 6; ++face) {
        gl_Layer = face; // встроенная переменная, указывающая на то, какую грань мы рендерим
        for(int i = 0; i < 3; ++i) // для каждой вершины треугольника
        {
            FragPos = gl_in[i].gl_Position;
            gl_Position = shadowMatrices[face] * FragPos;
            EmitVertex();
        }    
        EndPrimitive();
    }
}

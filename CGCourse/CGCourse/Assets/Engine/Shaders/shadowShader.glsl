#shader vertex
#version 460 core

layout (location = 0) in vec3 geo_Pos;

uniform mat4 lightSpaceMatrix;

void main() {
    gl_Position = lightSpaceMatrix * vec4(geo_Pos, 1.0);
}

#shader fragment
#version 460 core

/* Light information sent by the engine */
layout(std430, binding = 0) buffer LightSSBO
{
    mat4 ssbo_Lights[];
};

void main(){
    // gl_FragDepth = gl_FragCoord.z;
}

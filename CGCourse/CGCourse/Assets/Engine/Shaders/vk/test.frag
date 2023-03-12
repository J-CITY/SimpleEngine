#version 460 core

/* Information passed from the fragment shader */
layout(location = 0) in VS_OUT {
    vec3        FragPos;
    vec3        Normal;
    vec2        TexCoords;
    mat3        TBN;
    flat vec3   TangentViewPos;
    vec3        TangentFragPos;

    vec4 FragPosDirLightSpace;
    vec4 FragPosSpotLightSpace;

    vec4 EyeSpacePosition;
} fs_in;

//OUTS
layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;
layout (location = 3) out vec4 gRoughAO;

void main() {
    gPosition = vec4(1,0,0,1);
    gNormal = vec4(0,1,0,1);
    gAlbedoSpec = vec4(0,0,1,1);
    gRoughAO = vec4(1,1,0,1);
}

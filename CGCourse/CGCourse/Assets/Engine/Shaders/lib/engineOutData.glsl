/* Information passed to the fragment shader */
out VS_OUT {
    vec3        FragPos;
    vec3        Normal;
    vec2        TexCoords;
    mat3        TBN;
    flat vec3   TangentViewPos;
    vec3        TangentFragPos;

    vec4 FragPosDirLightSpace;
    vec4 FragPosSpotLightSpace;
    
    vec4 EyeSpacePosition;

    vec4 newPos;
    vec4 prevPos;
} vs_out;

struct LightOGL {
    float pos[3];
    float cutoff;
    float forward[3];
    float outerCutoff;
    float color[3];
    float constant;
    int type;
    float linear;
    float quadratic;
    float intensity;
    float radius;
    vec3 padding;
};

layout(std430, binding = 0) buffer LightSSBO {
    LightOGL ssbo_Lights[];
};

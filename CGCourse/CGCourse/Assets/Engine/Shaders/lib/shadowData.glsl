//layout(std140, binding = 2) 
uniform struct EngineShadowUBO {
//dir light
    mat4 dirMatrices[16];
    mat4 dirLightSpaceMatrix;
    float dirCascadePlaneDistances[16];
    int dirCascadeCount;
    vec3 dirLightDir;
    vec3 dirLightPos;
    float dirFarPlane;
//PCSS light
    int   dirBLOCKER_SEARCH_NUM_SAMPLES;//64
    int   dirPCF_NUM_SAMPLES ;//64;
    float dirNEAR_PLANE ;//0.05;
    float dirLIGHT_WORLD_SIZE ;// .5;
    float dirLIGHT_FRUSTUM_WIDTH; //3.75; 
    float dirLIGHT_SIZE; //(LIGHT_WORLD_SIZE / LIGHT_FRUSTUM_WIDTH);
//spot light
    mat4 spotLightSpaceMatrix;

    bool useDirLightShadow;
    bool useDirBakedLightShadow;
    bool useSpotLightShadow;
    bool usePointLightShadow;
} engine_ShadowUBO;

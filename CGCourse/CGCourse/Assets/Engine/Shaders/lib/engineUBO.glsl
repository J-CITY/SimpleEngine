
layout (std140, binding = 0) uniform EngineUBO {
    mat4    View;
    mat4    Projection;
    vec3    ViewPos;
    float   Time;
    vec2    ViewportSize;
    float    FPS;
    int      FrameCount;
} engine_UBO;

struct PushModel {
    mat4 model;
};
uniform PushModel engine_Model;


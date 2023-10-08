#version 460 core

out vec4 FragColor;

in vec2 TexCoords;

//layout (std140) uniform EngineUBO
//{
//    mat4    ubo_Model;
//    mat4    ubo_View;
//    mat4    ubo_Projection;
//    vec3    ubo_ViewPos;
//    float   ubo_Time;
//    vec2    ubo_ViewportSize;
//};

uniform sampler2D u_Scene;
uniform vec3 u_ChromaticAbberationParams;

vec3 applyChromaticAbberation(vec3 color, vec2 texcoord, sampler2D inputTex, float minDistance, float intensity, float distortion) {
    vec2 chromaticAberrationOffset = 2.0f * texcoord - 1.0f;
    float chromaticAberrationOffsetLength = length(chromaticAberrationOffset);
    chromaticAberrationOffsetLength *= distortion;
    float chromaticAberrationTexel = chromaticAberrationOffsetLength - minDistance;

    bool applyChromaticAberration = chromaticAberrationTexel > 0.0f;
    if (applyChromaticAberration)
    {
        chromaticAberrationTexel *= chromaticAberrationTexel;
        chromaticAberrationOffsetLength = max(chromaticAberrationOffsetLength, 0.0001f);

        float multiplier = chromaticAberrationTexel / chromaticAberrationOffsetLength;

        chromaticAberrationOffset *= multiplier * intensity;

        vec2 offsetUV = texcoord - 2.0f * chromaticAberrationOffset;
        color.r = texture(inputTex, offsetUV).r;

        offsetUV = texcoord - chromaticAberrationOffset;
        color.g = texture(inputTex, offsetUV).g;
    }
    return color;
}

void main() {
    vec3 inputColor   = texture(u_Scene, TexCoords).rgb;
    float minDistance = u_ChromaticAbberationParams.x;
    float intensity   = u_ChromaticAbberationParams.y;
    float distortion  = u_ChromaticAbberationParams.z;
    vec3 outputColor  = applyChromaticAbberation(inputColor, TexCoords, u_Scene, minDistance, intensity, distortion);
    FragColor = vec4(outputColor, 1.0f);
}

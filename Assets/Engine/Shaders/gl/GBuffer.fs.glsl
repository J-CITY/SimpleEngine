#version 460 core
#include "../lib/engineUBO.glsl"
#include "../lib/engineInData.glsl"

//OUTS
layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gVelocity;
layout (location = 2) out vec4 gEyePosition;


void main() {
    //if (texture(u_AlbedoMap, g_TexCoords).a > 0.5) {
        gPosition = vec4(fs_in.FragPos, 1);
        gEyePosition = fs_in.EyeSpacePosition;
    //}
    vec2 newPos = ((fs_in.newPos.xy / fs_in.newPos.w) * 0.5 + 0.5);
    vec2 prePos = ((fs_in.prevPos.xy / fs_in.prevPos.w) * 0.5 + 0.5);
    vec2 velocity = (newPos - prePos);// * 10;
    gVelocity = vec4(velocity, 0, 1);
}

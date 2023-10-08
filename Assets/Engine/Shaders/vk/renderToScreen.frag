
#version 450

layout(set = 0, binding = 0) uniform sampler2D inputTexture;
layout(location = 0) out vec4 color;
layout(location = 0) in vec2 inUV;

void main() {
    vec3 fragPos = texture(inputTexture, inUV).rgb;
    //vec3 fragPos = texture(inputTexture, vec3(inUV,4)).rrr;
    color = vec4(fragPos, 1.0);
}

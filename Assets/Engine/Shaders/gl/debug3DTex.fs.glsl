#version 460 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2DArray depthMap;
uniform sampler3D depthMap3D;
uniform float near_plane;
uniform float far_plane;
uniform int layer;
uniform int isPerspective = 0;
uniform int isRGB = 0;
uniform int is3D = 0;

// required when using a perspective projection matrix
float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0; // Back to NDC 
    return (2.0 * near_plane * far_plane) / (far_plane + near_plane - z * (far_plane - near_plane));	
}

void main() {
    if (isRGB == 0) {
        float depthValue = 0.0;
        if (is3D == 0) {
            depthValue = texture(depthMap, vec3(TexCoords, layer)).r;
        }
        else {
            depthValue = texture(depthMap3D, vec3(TexCoords, layer)).r;
        }
        if (isPerspective == 1) {
            FragColor = vec4(vec3(LinearizeDepth(depthValue) / far_plane), 1.0); // perspective
        }
        else {
            FragColor = vec4(vec3(depthValue), 1.0); // orthographic
        }
    }
    else {
        vec3 depthValue = vec3(0.0);
        if (is3D == 0) {
            depthValue = texture(depthMap, vec3(TexCoords, layer)).rgb;
        }
        else {
            depthValue = texture(depthMap3D, vec3(TexCoords, layer)).rgb;
        }
        FragColor = vec4(depthValue, 1.0);
    }
}

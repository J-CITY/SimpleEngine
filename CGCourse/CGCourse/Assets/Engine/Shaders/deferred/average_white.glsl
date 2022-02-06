#shader vertex
#version 460 core

layout(location = 0) in vec4 position;

out vec2 TexCoord;

void main()
{
    gl_Position = position;
    TexCoord = position.xy * 0.5 + vec2(0.5);
}

#shader fragment
#version 460 core

in vec2 TexCoord;
out vec4 OutColor;

uniform sampler2D curFrameHDR;
uniform sampler2D prevFrameWhite;
uniform float adaptSpeed;
uniform float adaptThreshold;

vec3 luminance = vec3(0.2125f, 0.7154f, 0.0721f);

void main()
{
    vec3 color = texture(curFrameHDR, TexCoord).rgb;
    float oldWhite = texture(prevFrameWhite, vec2(0.0f)).r;
    float curWhite = dot(luminance, color);

    float diff = abs(curWhite - oldWhite) < adaptThreshold ? 0.0f : curWhite - oldWhite;

    float white = oldWhite + diff * adaptSpeed;
    white = isnan(white) ? 1.0f : white;
    OutColor = vec4(white, 0.0f, 0.0f, 1.0f);
}

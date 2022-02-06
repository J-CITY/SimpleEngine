#shader vertex
#version 460 core

layout(location = 0) in vec3 position;

uniform mat4 StaticViewProjection;
uniform mat3 Rotation;

out vec3 TexCoords;

void main()
{
    TexCoords = position;
    vec3 pos = Rotation * position * 10000.0f;
    vec4 screenPos = StaticViewProjection * vec4(pos, 1.0);
    gl_Position = screenPos.xyww;
}

#shader fragment
#version 460 core

out vec4 Color;
in vec3 TexCoords;

uniform samplerCube skybox;
uniform float gamma;
uniform float luminance;

void main()
{
    vec3 skyboxColor = texture(skybox, TexCoords).rgb;
    skyboxColor = pow(skyboxColor, vec3(gamma));
    skyboxColor = luminance * skyboxColor;
    Color = vec4(skyboxColor, 1.0f);
}

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

out vec4 Color;

uniform sampler2D albedoTex;
uniform float weight;

void main()
{
    vec4 albedoEmmision = texture(albedoTex, TexCoord).rgba;
    float emmision = albedoEmmision.a;
    vec3 albedo = albedoEmmision.rgb;
    emmision = emmision / (1.0f - emmision);
    Color = vec4(weight * emmision * albedo, 1.0f);
}

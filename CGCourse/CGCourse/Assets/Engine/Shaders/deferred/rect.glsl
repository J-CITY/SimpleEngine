#shader vertex
#version 460 core

layout(location = 0) in vec3 position;
layout (location = 1) in vec2 aTexCoords;

out vec2 TexCoord;

void main()
{
    //gl_Position = position;
    //TexCoord = aTexCoords;//position.xy * 0.5 + vec2(0.5);
    TexCoord = aTexCoords;
    gl_Position = vec4(position, 1.0);
}

#shader fragment
#version 460 core

out vec4 Color;
in vec2 TexCoord;
uniform sampler2D tex;

void main()
{
    Color = texture(tex, TexCoord).rgba;
}

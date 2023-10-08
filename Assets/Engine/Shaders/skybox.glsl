#shader vertex
#version 460 core

layout (location = 0) in vec3 Position;
 
uniform mat4 gWVP;
 
out vec3 TexCoord0;
 
void main()
{
    vec3 pos = Position * 10000.0f;
    gl_Position = gWVP * vec4(pos, 1);
    //vec4 WVP_Pos = gWVP * vec4(Position, 1.0);
    //gl_Position = WVP_Pos.xyww;
 
    TexCoord0 = Position;
}

#shader fragment
#version 460 core

in vec3 TexCoord0;
 
out vec4 FRAGMENT_COLOR;
 
uniform samplerCube gCubemapTexture;
 
void main()
{
    FRAGMENT_COLOR = texture(gCubemapTexture, TexCoord0);
}
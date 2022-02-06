#shader vertex
#version 460 core

layout (location = 0) in vec3 Position;
 
uniform mat4 gWVP;
 
out vec2 TexCoord0;
 
void main() {
    vec4 WVP_Pos = gWVP * vec4(Position, 1.0);
 
    gl_Position = WVP_Pos;
 
    TexCoord0 = vec2((Position.x+1.0)/2.0, 1 - (Position.y+1.0)/2.0);
}

#shader fragment
#version 460 core

in vec2 TexCoord0;
 
out vec4 FRAGMENT_COLOR;
 
uniform sampler2D gTex;
 
void main() {
    FRAGMENT_COLOR = texture(gTex, TexCoord0);
}
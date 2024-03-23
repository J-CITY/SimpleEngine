#version 120

attribute vec3 geo_Pos;

uniform mat4 engine_Model;
uniform mat4 engine_Projection;
uniform mat4 engine_View;

void main()
{
    gl_Position = engine_Projection * engine_View * engine_Model * vec4(geo_Pos, 1.0);
}

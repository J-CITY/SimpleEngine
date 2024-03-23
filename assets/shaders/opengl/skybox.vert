#version 120

//1. declare attributes
attribute vec3 geo_Pos;

//2. declare varying type which will transfer the texture coordinates to the fragment shader
varying vec3 vTexCoordinates;

uniform mat4 engine_Model;
uniform mat4 engine_Projection;
uniform mat4 engine_View;

void main() {
    //4. Generate the UV coordinates
    vTexCoordinates = normalize(geo_Pos.xyz);
    //5. transform every position vertex by the model-view-projection matrix
    gl_Position = engine_Projection * engine_View * engine_Model * vec4(geo_Pos, 1.0);
    //6. Trick to place the skybox behind any other 3D model
    //gl_Position = gl_Position.xyww;
}

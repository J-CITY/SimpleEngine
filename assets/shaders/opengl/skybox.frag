#version 120

//1. declare a uniform sampler2d that contains the texture data
uniform samplerCube u_SkyBoxTexture;

//2. declare varying type which will transfer the texture coordinates from the vertex shader
varying vec3 vTexCoordinates;

void main() {
    //3. set the final color to the output of the fragment shader
    gl_FragColor = textureCube(u_SkyBoxTexture, vTexCoordinates);
}

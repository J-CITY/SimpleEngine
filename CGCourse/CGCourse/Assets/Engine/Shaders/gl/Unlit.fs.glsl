#version 460 core

out vec4 FRAGMENT_COLOR;

in VS_OUT
{
    vec2 TexCoords;
} fs_in;

uniform vec4        u_Diffuse = vec4(1.0, 1.0, 1.0, 1.0);
uniform sampler2D   u_DiffuseMap;
uniform vec2        u_TextureTiling = vec2(1.0, 1.0);
uniform vec2        u_TextureOffset = vec2(0.0, 0.0);

void main()
{
    FRAGMENT_COLOR = texture(u_DiffuseMap, u_TextureOffset + vec2(mod(fs_in.TexCoords.x * u_TextureTiling.x, 1), mod(fs_in.TexCoords.y * u_TextureTiling.y, 1))) * u_Diffuse;
}
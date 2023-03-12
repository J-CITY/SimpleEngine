#version 460 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D u_Scene;

void main() {
  float amount = 0.3;

  vec2 texSize   = textureSize(u_Scene, 0).xy;
  vec2 fragCoord = gl_FragCoord.xy;

  float neighbor = amount * -1.0;
  float center   = amount *  4.0 + 1.0;

  vec3 color =
        texture(u_Scene, (fragCoord + vec2( 0,  1)) / texSize).rgb
      * neighbor

      + texture(u_Scene, (fragCoord + vec2(-1,  0)) / texSize).rgb
      * neighbor
      + texture(u_Scene, (fragCoord + vec2( 0,  0)) / texSize).rgb
      * center
      + texture(u_Scene, (fragCoord + vec2( 1,  0)) / texSize).rgb
      * neighbor

      + texture(u_Scene, (fragCoord + vec2( 0, -1)) / texSize).rgb
      * neighbor
      ;

  FragColor = vec4(color, texture(u_Scene, TexCoords).a);
}
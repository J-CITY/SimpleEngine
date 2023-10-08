#version 460 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D u_Scene;

uniform sampler2D positionTexture;
uniform sampler2D noiseTexture;
uniform sampler2D outOfFocusTexture;

uniform vec3 focusPoint;
uniform vec2 nearFar;

//god rays
//uniform vec3 sunPos;

//layout (std140) uniform EngineUBO
//{
//    mat4    ubo_Model;
//    mat4    ubo_View;
//    mat4    ubo_Projection;
//    vec3    ubo_ViewPos;
//    float   ubo_Time;
//    vec2    ubo_ViewportSize;
//};

void main() {
  float minDistance =  8.0;
  float maxDistance = 222.0;

  float far  = nearFar.y;

  vec2 texSize  = textureSize(u_Scene, 0).xy;
  //vec2 texCoord = gl_FragCoord.xy / texSize;

  vec4 focusColor = texture(u_Scene, TexCoords);

  FragColor = focusColor;

  vec4 position = texture(positionTexture, TexCoords);

  if (position.a <= 0) { FragColor = vec4(1.0); return; }

  vec4 outOfFocusColor = texture(outOfFocusTexture, TexCoords);
  vec4 _focusPoint      = texture(positionTexture,   focusPoint.xy);

  float blur =
    smoothstep
      ( minDistance
      , maxDistance
      , length(position - _focusPoint)
      );

  FragColor  = mix(focusColor, outOfFocusColor, blur);
}

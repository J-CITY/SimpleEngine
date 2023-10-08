#shader vertex
#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 TexCoords;

void main() {
    TexCoords = aTexCoords;
    gl_Position = vec4(aPos, 1.0);
}

#shader fragment
#version 460 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D u_Scene;
uniform vec2 gamma;

uniform sampler2D positionTexture;
uniform sampler2D noiseTexture;

uniform vec2 nearFar;


void main() {
  float minSeparation = 1.0;
  float maxSeparation = 1.0;
  float minDistance   = 1.5;
  float maxDistance   = 2.0;
  float noiseScale    = 1.0;
  int   size          = 1;
  vec3  colorModifier = vec3(0.522, 0.431, 0.349);

  //colorModifier = pow(colorModifier, vec3(gamma.x));

  float near = nearFar.x;
  float far  = nearFar.y;

  vec2 fragCoord = gl_FragCoord.xy;

  vec2 texSize   = textureSize(u_Scene, 0).xy;

  vec4  color        = texture(u_Scene,        TexCoords);

  FragColor = vec4(0.0);

  vec2 noise  = texture(noiseTexture, fragCoord / textureSize(noiseTexture, 0).xy).rb;
       noise  = noise * 2.0 - 1.0;
       noise *= noiseScale;

  vec2 texCoords = (fragCoord - noise) / texSize;

  vec4 position     = texture(positionTexture, texCoords);
  vec4 positionTemp = position;

  if (position.a <= 0.0) { position.y = far; }

  float depth =
    clamp
      (   1.0
        - ( (far - position.y)
          / (far - near)
          )
      , 0.0
      , 1.0
      );

  float separation = mix(maxSeparation, minSeparation, depth);
  float count      = 1.0;
  float mx         = 0.0;

  for (int i = -size; i <= size; ++i) {
    for (int j = -size; j <= size; ++j) {
      texCoords =
          (vec2(i, j) * separation + (fragCoord + noise))
        / texSize;

      positionTemp =
        texture
          ( positionTexture
          , texCoords
          );

      if (positionTemp.y <= 0.0) { positionTemp.y = far; }

      mx = max(mx, abs(position.y - positionTemp.y));
      count += 1.0;
    }
  }

  float diff         = smoothstep(minDistance, maxDistance, mx);

  texCoords = fragCoord / texSize;

  vec3 lineColor  = texture(u_Scene, texCoords).rgb;
       lineColor *= colorModifier;

  FragColor.rgb = mix(color.rgb, lineColor, clamp(diff, 0.0, 1.0));
  FragColor.a   = 1.0;
}
#version 460 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D u_Scene;
uniform sampler2D positionTexture;

void main() {
  float levels = 6.0;

  vec2 texSize  = textureSize(u_Scene, 0).xy;
  vec2 texCoord = TexCoords;

  // Avoid the background.
  vec4 position = texture(positionTexture, texCoord);
  if (position.a <= 0) { FragColor = texture(u_Scene, texCoord); return; }

  FragColor = texture(u_Scene, texCoord);

  //FragColor.rgb = pow(FragColor.rgb, vec3(gamma.y));

  float greyscale = max(FragColor.r, max(FragColor.g, FragColor.b));

  float lower     = floor(greyscale * levels) / levels;
  float lowerDiff = abs(greyscale - lower);

  float upper     = ceil(greyscale * levels) / levels;
  float upperDiff = abs(upper - greyscale);

  float level      = lowerDiff <= upperDiff ? lower : upper;
  float adjustment = level / greyscale;

  FragColor.rgb = FragColor.rgb * adjustment;

  //FragColor.rgb = pow(FragColor.rgb, vec3(gamma.x));
}

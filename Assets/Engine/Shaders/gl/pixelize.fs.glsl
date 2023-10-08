#version 460 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D u_Scene;
uniform sampler2D positionTexture;

uniform vec2 parameters;

void main() {
  // Must be odd.
  int pixelSize = int(parameters.x);

  vec2 texSize  = textureSize(u_Scene, 0).xy;
  
  // Avoid the background.
  vec4 position = texture(positionTexture, TexCoords);

  if (position.a <= 0.0) {
    //FragColor = texture(u_Scene, TexCoords);
    //return;
  }

  float x = int(gl_FragCoord.x) % pixelSize;
  float y = int(gl_FragCoord.y) % pixelSize;

        x = floor(pixelSize / 2.0) - x;
        y = floor(pixelSize / 2.0) - y;

        x = gl_FragCoord.x + x;
        y = gl_FragCoord.y + y;

  vec2 uv = vec2(x, y) / texSize;

  FragColor = texture(u_Scene, uv);
}

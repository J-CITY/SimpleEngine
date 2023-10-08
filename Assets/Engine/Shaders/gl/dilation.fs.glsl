#version 460 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D u_Scene;
uniform vec2 parameters;

void main() {
  int   size         = int(parameters.x);
  float separation   =     parameters.y;
  float minThreshold = 0.2;
  float maxThreshold = 0.5;

  vec2 texSize   = textureSize(u_Scene, 0).xy;

  FragColor = texture(u_Scene, TexCoords);
  vec2 fragCoord = gl_FragCoord.xy;

  if (size <= 0) { return; }

  float  mx = 0.0;
  vec4  cmx = FragColor;

  for (int i = -size; i <= size; ++i) {
    for (int j = -size; j <= size; ++j) {
      // For a rectangular shape.
      //if (false);

      // For a diamond shape;
      //if (!(abs(i) <= size - abs(j))) { continue; }

      // For a circular shape.
      if (!(distance(vec2(i, j), vec2(0, 0)) <= size)) { continue; }

      vec4 c =
        texture
          ( u_Scene
          ,   ( gl_FragCoord.xy
              + (vec2(i, j) * separation)
              )
            / texSize
          );

      float mxt = dot(c.rgb, vec3(0.3, 0.59, 0.11));

      if (mxt > mx) {
         mx = mxt;
        cmx = c;
      }
    }
  }

  FragColor.rgb =
    mix
      ( FragColor.rgb
      , cmx.rgb
      , smoothstep(minThreshold, maxThreshold, mx)
      );
}
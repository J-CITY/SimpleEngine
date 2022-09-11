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

layout (std140) uniform EngineUBO {
    mat4    ubo_Model;
    mat4    ubo_View;
    mat4    ubo_Projection;
    vec3    ubo_ViewPos;
    float   ubo_Time;
    vec2    ubo_ViewportSize;
};

uniform sampler2D u_Scene;

#define PI 3.1415926538

void main() {
  float amount = 0.1;

  vec2 texSize  = textureSize(u_Scene, 0).xy;
  
  vec4 color = texture(u_Scene, TexCoords);

    float randomIntensity =
      fract
        ( 10000
        * sin
            (
              ( gl_FragCoord.x
              + gl_FragCoord.y
              * ubo_Time
              )
            * PI / 180.0
            )
        );

    amount *= randomIntensity;

    color.rgb += amount;

  FragColor = color;
}
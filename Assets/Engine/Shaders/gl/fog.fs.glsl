#version 460 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D u_Scene;
uniform sampler2D positionTexture;

#include "../lib/fog.glsl"

uniform FogParameters u_engine_FogParams;

void main() {
    FragColor = texture(u_Scene, TexCoords);
	if(u_engine_FogParams.isEnabled) {
      vec4 ioEyeSpacePosition = texture(positionTexture, TexCoords);
      float fogCoordinate = abs(ioEyeSpacePosition.z / ioEyeSpacePosition.w);
      FragColor = mix(FragColor, vec4(u_engine_FogParams.color, 1.0), getFogFactor(u_engine_FogParams, fogCoordinate));
    }
}

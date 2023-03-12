#pragma once

import glmath;

namespace KUMA::RENDER {
	struct EngineUBO {
		MATHGL::Matrix4    View;
		MATHGL::Matrix4    Projection;
		MATHGL::Vector3    ViewPos;
		float   Time = 0.0f;
		MATHGL::Vector2f    ViewportSize;
		float FPS = 0.0f;
		int FrameCount = 0;
	};

	class GameRendererInterface {
	public:
		virtual ~GameRendererInterface() = default;
		virtual void renderScene() = 0;
	};
	
}

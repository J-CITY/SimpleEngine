#pragma once

import glmath;

namespace IKIGAI::RENDER {
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
		virtual void resize() {}; //TODO: make pure virtual
	};
	
}

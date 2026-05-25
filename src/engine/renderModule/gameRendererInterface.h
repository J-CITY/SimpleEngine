#pragma once
#include "mathModule/math.h"


namespace IKIGAI::RENDER {
	struct RenderGraphPipeline;

	struct EngineUBO {
		MATH::Matrix4f    View;
		MATH::Matrix4f    Projection;
		MATH::Vector3f    ViewPos;
		float   Time = 0.0f;
		MATH::Vector2f    ViewportSize;
		float FPS = 0.0f;
		int FrameCount = 0;
	};

	constexpr size_t MAX_BONES = 128;
	struct BonesUBO {
		int use = 0;
		alignas(16) MATH::Matrix4f bones[MAX_BONES];
	};

	struct EngineModel {
		MATH::Matrix4f    Model;
	};

	class GameRendererInterface {
	public:
		MATH::Vector2i viewPoreSize = MATH::Vector2i(800, 600);
		virtual ~GameRendererInterface() = default;
		virtual void renderScene() = 0;
		virtual void resize() {}; //TODO: make pure virtual=
		virtual const RenderGraphPipeline& getCurrentPipeline() const = 0;
	};
	
}

#pragma once
#include <map>
#include <memory>

#include "mathModule/math.h"
#include "utilsModule/weakPtr.h"

//#include <map>
//#include "backends/interface/meshInterface.h"
//#include "backends/interface/materialInterface.h"
#include <coreModule/ecs/components/skeletal.h>
#include <coreModule/ecs/components/cameraComponent.h>



namespace IKIGAI {
	namespace RENDER {
		class MeshInterface;
		class MaterialInterface;

		struct Drawable {
			MATH::Matrix4f prevWorld;
			MATH::Matrix4f world;
			std::shared_ptr<MeshInterface> mesh;
			std::shared_ptr<MaterialInterface> material;
			UTILS::WeakPtr<ECS::Skeletal> animator;
		};
		
		using OpaqueDrawables = std::multimap<float, Drawable, std::less<float>>;
		using TransparentDrawables = std::multimap<float, Drawable, std::greater<float>>;

		class FrameBufferInterface;
	}
}


namespace IKIGAI {
	namespace RENDER {
		struct RenderChunk {
			std::shared_ptr<FrameBufferInterface> frameBuffer;
			UTILS::WeakPtr<ECS::CameraComponent> camera;

			OpaqueDrawables opaqueDrawablesForward;
			TransparentDrawables transparentDrawablesForward;
			OpaqueDrawables opaqueDrawablesDeferred;
			TransparentDrawables transparentDrawablesDeferred;
		};
	}
}

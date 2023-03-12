#pragma once

#include <map>
import glmath;


#include "backends/interface/meshInterface.h"
#include "backends/interface/materialInterface.h"
#include "../../../ecs/components/skeletal.h"

namespace KUMA {
	namespace RENDER {
		class Material;

		struct Drawable {
			MATHGL::Matrix4 prevWorld;
			MATHGL::Matrix4 world;
			std::shared_ptr<MeshInterface> mesh;
			std::shared_ptr<MaterialInterface> material;
			std::optional<Ref<ECS::Skeletal>> animator;
		};

		using OpaqueDrawables = std::multimap<float, Drawable, std::less<float>>;
		using TransparentDrawables = std::multimap<float, Drawable, std::greater<float>>;
	}
}

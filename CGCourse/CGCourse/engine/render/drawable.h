#pragma once

#include <map>
import glmath;

namespace KUMA
{
	namespace RESOURCES
	{
		class Mesh;
	}

	namespace ECS
	{
		class Skeletal;
	}
}

namespace KUMA {
	namespace RENDER {
		class Material;

		struct Drawable {
			MATHGL::Matrix4 world;
			RESOURCES::Mesh* mesh;
			std::shared_ptr<RENDER::Material> material;
			std::shared_ptr<ECS::Skeletal> animator;
		};

		using OpaqueDrawables = std::multimap<float, Drawable, std::less<float>>;
		using TransparentDrawables = std::multimap<float, Drawable, std::greater<float>>;
	}
}

#pragma once

#include <map>
#include "../utils/math/Matrix4.h"

namespace KUMA
{
	namespace RESOURCES
	{
		class Mesh;
	}
}

namespace KUMA {
	namespace RENDER {
		class Material;

		struct Drawable {
			MATHGL::Matrix4 world;
			RESOURCES::Mesh* mesh;
			std::shared_ptr<RENDER::Material> material;
		};

		using OpaqueDrawables = std::multimap<float, Drawable, std::less<float>>;
		using TransparentDrawables = std::multimap<float, Drawable, std::greater<float>>;
	}
}

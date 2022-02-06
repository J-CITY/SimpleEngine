#pragma once
#include "../../utils/math/Vector3.h"

namespace KUMA {
	namespace RENDER {
		struct BoundingSphere {
			MATHGL::Vector3 position;
			float radius;
		};
	}
}

#pragma once
#include "mathModule/math.h"

namespace IKIGAI {
	namespace RENDER {
		struct BoundingSphere {
			MATH::Vector3f position;
			float radius = 0.0f;

			[[nodiscard]] float calcGrowth(const BoundingSphere& sphere) const;
			[[nodiscard]] float getSize() const;
			[[nodiscard]] int intersect(const BoundingSphere& sphere) const;
			void createFromTwo(const BoundingSphere& one, const BoundingSphere& two);
		};
	}
}

#pragma once
import glmath;

namespace KUMA {
	namespace RENDER {
		struct BoundingSphere {
			MATHGL::Vector3 position;
			float radius;

			[[nodiscard]] float calcGrowth(const BoundingSphere& sphere) const;
			[[nodiscard]] float getSize() const;
			[[nodiscard]] int intersect(const BoundingSphere& sphere) const;
			void createFromTwo(const BoundingSphere& one, const BoundingSphere& two);
		};
	}
}

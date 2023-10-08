#pragma once
import glmath;

namespace IKIGAI {
	namespace RENDER {
		struct BoundingSphere {
			MATHGL::Vector3 position;
			float radius = 0.0f;

			[[nodiscard]] float calcGrowth(const BoundingSphere& sphere) const;
			[[nodiscard]] float getSize() const;
			[[nodiscard]] int intersect(const BoundingSphere& sphere) const;
			void createFromTwo(const BoundingSphere& one, const BoundingSphere& two);
		};
	}
}

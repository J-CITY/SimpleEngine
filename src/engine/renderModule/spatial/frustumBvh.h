#pragma once
#include "renderModule/frustum.h"
#include "utilsModule/spatial/bvh.h"

namespace IKIGAI::RENDER {
struct SphereBvhTraits {
	static constexpr std::size_t dimensions = 3;
	static BoundingSphere merge(const BoundingSphere& a, const BoundingSphere& b) {
		BoundingSphere r;
		r.createFromTwo(a, b);
		return r;
	}
	static float center(const BoundingSphere& v, std::size_t axis) {
		return v.position[axis];
	}
	static void validate(const BoundingSphere& v) {
		if (!std::isfinite(v.radius) || v.radius < 0)
			throw std::invalid_argument("Invalid sphere bounds");
		for (int i = 0; i < 3; ++i)
			if (!std::isfinite(v.position[i]))
				throw std::invalid_argument("Invalid sphere center");
	}
};
template <class Payload> using FrustumBvh = UTILS::SPATIAL::Bvh<BoundingSphere, Payload, SphereBvhTraits>;
inline auto frustumPredicate(const Frustum& frustum) {
	return [&frustum](const BoundingSphere& bounds) {
		using Result = UTILS::SPATIAL::Intersection;
		switch (frustum.boundingSphereInFrustumCollide(bounds)) {
		case COLLISION::TYPE::OUTSIDE:
			return Result::Outside;
		case COLLISION::TYPE::INSIDE:
			return Result::Inside;
		default:
			return Result::Intersect;
		}
	};
}
}

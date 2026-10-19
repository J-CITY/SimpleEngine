#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <stdexcept>

namespace IKIGAI::MATH::GEOMETRY {
template <std::size_t N, class Scalar = float> struct Aabb {
	static_assert(N > 0);
	std::array<Scalar, N> min { }, max { };
	void validate() const {
		for (std::size_t i = 0; i < N; ++i)
			if (!std::isfinite(min[i]) || !std::isfinite(max[i]) || min[i] > max[i])
				throw std::invalid_argument("Invalid spatial bounds");
	}
	bool overlaps(const Aabb& b) const {
		for (std::size_t i = 0; i < N; ++i)
			if (min[i] > b.max[i] || max[i] < b.min[i])
				return false;
		return true;
	}
	static Aabb merge(const Aabb& a, const Aabb& b) {
		Aabb r;
		for (std::size_t i = 0; i < N; ++i) {
			r.min[i] = std::min(a.min[i], b.min[i]);
			r.max[i] = std::max(a.max[i], b.max[i]);
		}
		return r;
	}
};
template <std::size_t N> struct AabbTraits {
	using Volume = Aabb<N>;
	static Volume merge(const Volume& a, const Volume& b) {
		return Volume::merge(a, b);
	}
	static float center(const Volume& v, std::size_t axis) {
		return v.min[axis] * 0.5f + v.max[axis] * 0.5f;
	}
	static void validate(const Volume& v) {
		v.validate();
	}
};
}

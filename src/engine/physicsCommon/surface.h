#pragma once
#include "forces.h"
#include <array>
#include <span>
#include <vector>

namespace IKIGAI::PHYSICS_COMMON {
template <class V> struct Surface {
	std::vector<V> positions, normals;
	std::vector<std::uint32_t> indices;
	V center { };
	float radius = 0;
};
// Renderer-independent snapshot with area-weighted normals and current bounds.
template <class Particle> auto surface(std::span<const Particle> particles, std::span<const std::array<std::size_t, 3>> triangles) {
	using V = decltype(Particle::position);
	Surface<V> result;
	result.positions.reserve(particles.size());
	result.normals.resize(particles.size());
	for (auto p : particles) {
		result.positions.push_back(p.position);
		result.center += p.position;
	}
	if (particles.empty())
		return result;
	result.center = result.center * (1.0f / static_cast<float>(particles.size()));
	for (auto t : triangles) {
		for (auto i : t)
			if (i >= particles.size() || i > UINT32_MAX)
				throw std::invalid_argument("Invalid physics surface index");
		auto normal = (particles[t[1]].position - particles[t[0]].position).cross(particles[t[2]].position - particles[t[0]].position);
		for (auto i : t) {
			result.normals[i] += normal;
			result.indices.push_back(static_cast<std::uint32_t>(i));
		}
	}
	for (std::size_t i = 0; i < particles.size(); ++i) {
		float length = std::sqrt(PHYSICS_COMMON::dot(result.normals[i], result.normals[i]));
		if (length > 1e-12f)
			result.normals[i] = result.normals[i] * (1 / length);
		auto delta = particles[i].position - result.center;
		result.radius = std::max(result.radius, std::sqrt(PHYSICS_COMMON::dot(delta, delta)));
	}
	return result;
}
}

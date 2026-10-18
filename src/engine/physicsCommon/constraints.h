#pragma once
#include "forces.h"
#include <span>
#include <vector>

namespace IKIGAI::PHYSICS_COMMON {
struct DistanceConstraint {
	std::size_t a = 0, b = 0;
	float restLength = 1;
	float compliance = 0;
};
// Lambda belongs to a constraint for one substep, and is reset at its start.
inline float multiplier(float error, float weightedGradient, float compliance, float dt, float& lambda) {
	float alpha = compliance / (dt * dt);
	if (weightedGradient < 1e-12f)
		return 0;
	float delta = (-error - alpha * lambda) / (weightedGradient + alpha);
	lambda += delta;
	return delta;
}
template <class Particle> void solveDistance(std::vector<Particle>& particles, const DistanceConstraint& c, float dt, float& lambda) {
	auto& a = particles[c.a];
	auto& b = particles[c.b];
	auto delta = b.position - a.position;
	float distance = std::sqrt(PHYSICS_COMMON::dot(delta, delta));
	if (distance < 1e-8f)
		return;
	float dl = multiplier(distance - c.restLength, a.inverseMass + b.inverseMass, c.compliance, dt, lambda);
	auto correction = delta * (dl / distance);
	a.position -= correction * a.inverseMass;
	b.position += correction * b.inverseMass;
}
}

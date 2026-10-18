#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <functional>
#include <map>
#include <stdexcept>

namespace IKIGAI::PHYSICS_COMMON {
using Id = std::uint64_t;
enum ForceTarget : unsigned { Rigid = 1, Soft = 2, Cloth = 4, Fluid = 8, All = 15 };
template <class V> float dot(V a, V b) {
	float result = a.x * b.x + a.y * b.y;
	if constexpr (requires { a.z; })
		result += a.z * b.z;
	return result;
}
template <class V> bool finite(V v) {
	bool result = std::isfinite(v.x) && std::isfinite(v.y);
	if constexpr (requires { v.z; })
		result = result && std::isfinite(v.z);
	return result;
}

// Values are world units, seconds and force units. Acceleration is mass independent.
// Wind is a medium velocity; drag uses (medium velocity - point velocity).
template <class V> struct ForceField {
	enum class Kind { Force, Acceleration, Drag, Wind, Buoyancy, Radial };
	Kind kind = Kind::Wind;
	Id id = 0, target = 0; // zero selects all matching objects in this world
	unsigned targets = All;
	std::uint32_t mask = ~0u;
	bool enabled = true, wake = true;
	V vector {}, center {}, localPoint {};
	float radius = 0; // zero is unbounded
	float linear = 0, quadratic = 1, area = 1, density = 1.225f;
	float waterHeight = 0, maxDepth = 0.5f, volume = 1, gravity = 9.81f;
	float gustAmplitude = 0, gustFrequency = 1;
	std::uint32_t seed = 0;
	std::function<V(V, double)> velocity; // optional spatial medium velocity

	void validate() const {
		auto nonnegative = [](float x) { return std::isfinite(x) && x >= 0; };
		if (kind < Kind::Force || kind > Kind::Radial || !PHYSICS_COMMON::finite(vector) || !PHYSICS_COMMON::finite(center)
			|| !PHYSICS_COMMON::finite(localPoint) || !nonnegative(radius) || !nonnegative(linear) || !nonnegative(quadratic) || !nonnegative(area)
			|| !nonnegative(density) || !nonnegative(volume) || !nonnegative(gravity) || !std::isfinite(waterHeight) || !std::isfinite(maxDepth)
			|| maxDepth <= 0 || !nonnegative(gustAmplitude) || !nonnegative(gustFrequency) || !(targets & All) || (targets & ~All))
			throw std::invalid_argument("Invalid physics force field");
	}
	V evaluate(V p, V pointVelocity, float inverseMass, double time) const {
		if (!enabled || inverseMass <= 0)
			return V {};
		V delta = p - center;
		float distance2 = PHYSICS_COMMON::dot(delta, delta);
		if (radius > 0 && distance2 > radius * radius)
			return V {};
		switch (kind) {
		case Kind::Force:
			return vector;
		case Kind::Acceleration:
			return vector * (1 / inverseMass);
		case Kind::Buoyancy: {
			float fraction = std::clamp((waterHeight + maxDepth - p.y) / (2 * maxDepth), 0.0f, 1.0f);
			V force {};
			force.y = density * volume * gravity * fraction;
			return force;
		}
		case Kind::Radial: {
			float distance = std::sqrt(distance2);
			return distance > 1e-6f ? delta * (linear / distance) : V {};
		}
		case Kind::Drag:
		case Kind::Wind: {
			V medium = velocity ? velocity(p, time) : vector;
			if (!PHYSICS_COMMON::finite(medium))
				throw std::invalid_argument("Nonfinite medium velocity");
			if (kind == Kind::Wind) {
				double phase = (seed % 65536u) * (6.283185307179586 / 65536);
				medium = medium * static_cast<float>(1 + gustAmplitude * std::sin(6.283185307179586 * gustFrequency * time + phase));
			}
			V relative = medium - pointVelocity;
			float speed = std::sqrt(PHYSICS_COMMON::dot(relative, relative));
			return relative * (linear + 0.5f * density * quadratic * area * speed);
		}
		}
		return V {};
	}
};

template <class V> class ForceRegistry {
	Id mNext = 1;
	std::map<Id, ForceField<V>> mFields;

public:
	Id add(const ForceField<V>& def) {
		def.validate();
		auto f = def;
		f.id = mNext++;
		mFields.emplace(f.id, f);
		return f.id;
	}
	void remove(Id id) {
		mFields.erase(id);
	}
	void removeTarget(Id target) {
		if (target)
			std::erase_if(mFields, [=](auto& e) { return e.second.target == target; });
	}
	void clear() {
		mFields.clear();
	} // IDs deliberately never reused
	ForceField<V>* field(Id id) {
		auto i = mFields.find(id);
		return i == mFields.end() ? nullptr : &i->second;
	}
	const auto& fields() const {
		return mFields;
	}
	V force(Id target, unsigned kind, std::uint32_t layer, V p, V velocity, float inverseMass, double time, bool* wake = nullptr) const {
		V result {};
		for (auto& [id, field] : mFields) {
			if ((field.target && field.target != target) || !(field.targets & kind) || !(field.mask & layer))
				continue;
			V f = field.evaluate(p, velocity, inverseMass, time);
			result += f;
			if (wake && field.wake && PHYSICS_COMMON::dot(f, f) > 1e-12f)
				*wake = true;
		}
		return result;
	}
};
}

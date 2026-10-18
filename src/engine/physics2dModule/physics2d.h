#pragma once

// This header deliberately has no ECS, renderer or third-party dependencies.
// Positions use world units, time uses seconds, angles use radians (XY, +Z CCW).
#include "collisionPipeline.h"
#include "physicsCommon/constraints.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <functional>
#include <map>
#include <span>
#include <vector>

namespace IKIGAI::PHYSICS2D {
using Id = std::uint64_t;
using COLLISION::cross;
using COLLISION::dot;
using COLLISION::Epsilon;
using COLLISION::finite;
using COLLISION::length;
using COLLISION::normalized;
using COLLISION::perpendicular;
using COLLISION::Pi;
using COLLISION::rotate;
using COLLISION::Vec2;
using Aabb = COLLISION::Aabb;
using Shape = COLLISION::Shape2D;
enum class BodyType { Static, Dynamic };
struct BodyDef {
	Shape shape = Shape::circle(0.5f);
	BodyType type = BodyType::Dynamic;
	Vec2 position, velocity;
	float angle = 0, angularVelocity = 0;
	float mass = 1, friction = 0.5f, restitution = 0.1f, gravityScale = 1;
	float linearDamping = 0.05f, angularDamping = 0.05f;
	bool allowSleep = true;
	std::uint32_t layer = 1, mask = ~0u;
};
struct Body : BodyDef {
	Id id = 0;
	Vec2 force;
	float torque = 0, inverseMass = 0, inverseInertia = 0, sleepTime = 0;
	bool sleeping = false;
	Aabb bounds() const;
};
struct ContactPoint {
	Vec2 position;
	float normalImpulse = 0, tangentImpulse = 0, bounce = 0;
};
struct Contact {
	Id a = 0, b = 0;
	Vec2 normal; // Always points from A towards B.
	float penetration = 0;
	std::array<ContactPoint, 2> points {};
	int count = 0;
};
bool collide(const Body& a, const Body& b, Contact& result);

struct JointDef {
	enum class Kind { Distance, Revolute };
	Kind kind = Kind::Distance;
	Id a = 0, b = 0;
	Vec2 localAnchorA, localAnchorB;
	float distance = 1;
	bool collideConnected = false;
};
struct Joint : JointDef {
	Id id = 0;
};
struct Particle {
	Vec2 position, velocity;
	float inverseMass = 1;
};
using DistanceConstraint = PHYSICS_COMMON::DistanceConstraint;
using ForceField = PHYSICS_COMMON::ForceField<Vec2>;
using ForceRegistry = PHYSICS_COMMON::ForceRegistry<Vec2>;
using ForceFieldRegistry = ForceRegistry;
struct SoftBodyDef {
	std::vector<Particle> particles;
	std::vector<DistanceConstraint> links;
	// A simple CCW boundary loop. Empty means no area-preserving constraint.
	std::vector<std::size_t> boundary;
	float radius = 0.08f, stiffness = 0.95f, areaStiffness = 0.8f, damping = 0.5f;
	bool selfCollision = false;
	bool cloth = false;
	// Negative selects the legacy stiffness path; nonnegative enables XPBD.
	float compliance = -1, areaCompliance = -1;
	std::uint32_t layer = 1;
};
struct SoftBody : SoftBodyDef {
	Id id = 0;
	float restArea = 0;
};
struct ClothDef {
	int width = 12, height = 12;
	float spacing = 0.1f, particleMass = 0.01f, radius = 0.02f;
	float stretchCompliance = 0, shearCompliance = 1e-5f, bendCompliance = 0.001f;
	Vec2 position;
	bool pinTop = true, selfCollision = true;
};
struct FluidDef {
	std::vector<Particle> particles;
	float radius = 0.05f, smoothingRadius = 0.2f;
	float particleMass = 1, restDensity = 100, viscosity = 0.1f;
	// The denominator has units: this is a numerical regularizer, not density.
	float relaxation = 0.01f;
	std::uint32_t layer = 1;
};
struct Fluid : FluidDef {
	Id id = 0;
};
struct Settings {
	Vec2 gravity { 0, -9.81f };
	float cellSize = 2, contactSlop = 0.003f;
	int substeps = 2, velocityIterations = 8, positionIterations = 4, particleIterations = 5;
	bool sleeping = true;
	float sleepLinearSpeed = 0.03f, sleepAngularSpeed = 0.03f, sleepDelay = 0.6f;
};
struct DebugDraw {
	// Colours are portable RGBA values; the caller chooses how to render them.
	std::function<void(Vec2, Vec2, std::uint32_t)> line;
	std::function<void(Vec2, float, std::uint32_t)> circle;
};

class World {
public:
	World();
	explicit World(Settings settings, CollisionPipeline pipeline = {});
	explicit World(CollisionPipeline pipeline)
		: World(Settings {}, std::move(pipeline)) { }
	const CollisionPipeline& collisionPipeline() const {
		return mPipeline;
	}
	Settings settings;
	ForceRegistry forces;
	double simulationTime() const {
		return mTime;
	}
	Id createBody(const BodyDef& def);
	Id createJoint(const JointDef& def);
	Id createSoftBody(const SoftBodyDef& def);
	Id createCloth(const ClothDef& def);
	SoftBody* cloth(Id id) {
		auto* s = softBody(id);
		return s && s->cloth ? s : nullptr;
	}
	void destroyCloth(Id id) {
		destroySoftBody(id);
	}
	Id createFluid(const FluidDef& def);
	void destroyBody(Id id); // Also removes joints referencing this body.
	void destroyJoint(Id id);
	void destroySoftBody(Id id) {
		forces.removeTarget(id);
		mSoft.erase(id);
	}
	void destroyFluid(Id id) {
		forces.removeTarget(id);
		mFluids.erase(id);
	}
	void clear();
	Body* body(Id id);
	SoftBody* softBody(Id id);
	Fluid* fluid(Id id);
	const auto& bodies() const {
		return mBodies;
	}
	const auto& joints() const {
		return mJoints;
	}
	const auto& softBodies() const {
		return mSoft;
	}
	const auto& fluids() const {
		return mFluids;
	}
	const auto& contacts() const {
		return mContacts;
	}
	void wake(Id id); // Wakes the whole contact/joint island.
	void applyForce(Id id, Vec2 force, Vec2 worldPoint);
	void applyImpulse(Id id, Vec2 impulse, Vec2 worldPoint);
	void teleport(Id id, Vec2 position, float angle);
	void setVelocity(Id id, Vec2 velocity, float angularVelocity = 0);
	void step(float dt);
	void debugDraw(const DebugDraw& draw) const;

private:
	// Monotonic IDs never reuse a removed body's ID, so stale handles stay stale.
	CollisionPipeline mPipeline;
	bool detect(const Body& a, const Body& b, Contact& out) const;
	Id mNextId = 1;
	double mTime = 0;
	std::map<Id, Body> mBodies;
	std::map<Id, Joint> mJoints;
	std::map<Id, SoftBody> mSoft;
	std::map<Id, Fluid> mFluids;
	std::vector<Contact> mContacts;
	void findContacts();
	void solveContacts();
	void solveJoints(float dt, bool position);
	void updateSleeping(float dt);
	void stepParticles(float dt);
	void projectParticle(Particle& p, float radius, float dt, UTILS::SPATIAL::ISpatialIndex<2>& solids);
};
}

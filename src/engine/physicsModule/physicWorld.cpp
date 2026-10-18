#include "physicWorld.h"
#include "spatialGrid.h"
#include <algorithm>
#include <cmath>
#include <set>
#include <stdexcept>

namespace IKIGAI::PHYSICS {
namespace {
	void require(bool ok, const char* message) {
		if (!ok)
			throw std::invalid_argument(message);
	}
	bool finite(Vec3 v) {
		return PHYSICS_COMMON::finite(v);
	}
	bool finite(MATH::QuaternionF q) {
		return std::isfinite(q.x) && std::isfinite(q.y) && std::isfinite(q.z) && std::isfinite(q.w)
			&& q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w > 1e-12f;
	}
	unsigned capacity(unsigned count) {
		require(count > 0 && count <= 1048576, "Invalid contact capacity");
		return count;
	}
}
PhysicWorld::PhysicWorld(unsigned count, unsigned iterations)
	: mContacts(capacity(count))
	, mResolver(iterations ? iterations : 12) {
	if (iterations)
		settings.velocityIterations = settings.positionIterations = static_cast<int>(iterations);
}
PhysicWorld::PhysicWorld(Settings config, CollisionPipeline pipeline, unsigned count)
	: settings(config)
	, mPipeline(std::move(pipeline))
	, mContacts(capacity(count))
	, mResolver(12) {
	mPipeline.validate();
}
COLLISION::Manifold3D PhysicWorld::detect(const COLLISION::ShapeView3D& a, const COLLISION::ShapeView3D& b) const {
	COLLISION::Manifold3D m;
	auto status = mPipeline.narrowPhase->collide(a, b, m);
	if (status == COLLISION::Status::Unsupported)
		throw std::runtime_error("Narrow phase does not support shape pair");
	if (status == COLLISION::Status::Separated)
		return {};
	if (m.count < 1 || m.count > 4 || !finite(m.normal) || std::abs(m.normal.squareMagnitude() - 1) > 0.001f)
		throw std::runtime_error("Invalid 3D collision manifold");
	for (int i = 0; i < m.count; ++i)
		if (!finite(m.points[i].position) || !std::isfinite(m.points[i].penetration) || m.points[i].penetration < 0)
			throw std::runtime_error("Invalid 3D contact point");
	return m;
}
Body* PhysicWorld::body(Id id) {
	auto i = mBodies.find(id);
	return i == mBodies.end() ? nullptr : &i->second;
}
SoftBody* PhysicWorld::softBody(Id id) {
	auto i = mSoft.find(id);
	return i == mSoft.end() ? nullptr : &i->second;
}
Fluid* PhysicWorld::fluid(Id id) {
	auto i = mFluids.find(id);
	return i == mFluids.end() ? nullptr : &i->second;
}
Id PhysicWorld::createBody(const BodyDef& d) {
	require(finite(d.position) && finite(d.velocity) && finite(d.angularVelocity) && finite(d.acceleration) && finite(d.orientation),
		"Nonfinite body state");
	require(d.type == BodyType::Static || d.type == BodyType::Dynamic, "Invalid body type");
	require(std::isfinite(d.mass) && d.mass > 0 && std::isfinite(d.friction) && d.friction >= 0 && std::isfinite(d.restitution) && d.restitution >= 0
			&& d.restitution <= 1 && std::isfinite(d.gravityScale) && std::isfinite(d.linearDamping) && d.linearDamping >= 0
			&& std::isfinite(d.angularDamping) && d.angularDamping >= 0,
		"Invalid body material");
	require(d.shape.kind == Shape::Kind::Sphere || d.shape.kind == Shape::Kind::Box, "Invalid shape kind");
	Body b;
	b.shape = d.shape.kind == Shape::Kind::Sphere ? Shape::sphere(d.shape.radius) : Shape::box(d.shape.halfSize);
	b.id = mNextId++;
	b.type = d.type;
	b.friction = d.friction;
	b.restitution = d.restitution;
	b.gravityScale = d.gravityScale;
	b.layer = d.layer;
	b.mask = d.mask;
	b.extraAcceleration = d.acceleration;
	b.setPosition(d.position);
	b.setOrientation(d.orientation);
	bool dynamic = d.type == BodyType::Dynamic;
	b.setInverseMass(dynamic ? 1 / d.mass : 0);
	b.setVelocity(dynamic ? d.velocity : Vec3 {});
	b.setRotation(dynamic ? d.angularVelocity : Vec3 {});
	b.setDamping(std::exp(-d.linearDamping), std::exp(-d.angularDamping));
	b.setCanSleep(d.allowSleep);
	b.setAwake(true);
	if (dynamic) {
		Vec3 inertia;
		if (d.shape.kind == Shape::Kind::Sphere)
			inertia = Vec3(0.4f * d.mass * d.shape.radius * d.shape.radius);
		else {
			auto h = d.shape.halfSize;
			inertia = Vec3(h.y * h.y + h.z * h.z, h.x * h.x + h.z * h.z, h.x * h.x + h.y * h.y) * (d.mass / 3);
		}
		b.setInertiaTensor(MATH::Matrix3f(inertia.x, 0, 0, 0, inertia.y, 0, 0, 0, inertia.z));
	}
	b.calculateDerivedData();
	mBodies.emplace(b.id, b);
	return b.id;
}
void PhysicWorld::destroyBody(Id id) {
	mPipeline.broadPhase->remove(id);
	// Removing a support may invalidate any resting contact island.
	for (auto& [bid, b] : mBodies)
		if (b.type == BodyType::Dynamic)
			b.setAwake(true);
	std::erase_if(mAttachments, [=](auto& e) { return e.second.body == id; });
	forces.removeTarget(id);
	mContactCount = 0;
	mBodies.erase(id);
}
void PhysicWorld::destroySoftBody(Id id) {
	std::erase_if(mAttachments, [=](auto& e) { return e.second.soft == id; });
	forces.removeTarget(id);
	mSoft.erase(id);
}
void PhysicWorld::destroyFluid(Id id) {
	forces.removeTarget(id);
	mFluids.erase(id);
}
void PhysicWorld::destroyAttachment(Id id) {
	mAttachments.erase(id);
}
void PhysicWorld::clear() {
	mPipeline.broadPhase->reset(settings.cellSize);
	mContactCount = 0;
	mAttachments.clear();
	mBodies.clear();
	mSoft.clear();
	mFluids.clear();
	forces.clear();
	mTime = 0;
}
void PhysicWorld::wake(Id id) {
	std::set<Id> seen;
	std::vector<Id> pending { id };
	while (!pending.empty()) {
		auto current = pending.back();
		pending.pop_back();
		if (!seen.insert(current).second)
			continue;
		auto* b = body(current);
		if (!b)
			continue;
		if (b->type == BodyType::Static && current != id)
			continue;
		if (b->type == BodyType::Dynamic)
			b->setAwake(true);
		for (auto c : contacts())
			for (int i = 0; i < 2; ++i)
				if (c.body[i] == b && c.body[1 - i])
					for (auto& [other, member] : mBodies)
						if (&member == c.body[1 - i])
							pending.push_back(other);
	}
}
void PhysicWorld::applyForce(Id id, Vec3 force, Vec3 point) {
	require(finite(force) && finite(point), "Nonfinite force");
	if (auto* b = body(id); b && b->type == BodyType::Dynamic) {
		b->force += force;
		b->torque += (point - b->getPosition()).cross(force);
		if (force.squareMagnitude() > 0)
			wake(id);
	}
}
void PhysicWorld::applyImpulse(Id id, Vec3 impulse, Vec3 point) {
	require(finite(impulse) && finite(point), "Nonfinite impulse");
	if (auto* b = body(id); b && b->type == BodyType::Dynamic) {
		b->addVelocity(impulse * b->getInverseMass());
		b->addRotation(b->getInverseInertiaTensorWorld().transform((point - b->getPosition()).cross(impulse)));
		wake(id);
	}
}
void PhysicWorld::teleport(Id id, Vec3 p, MATH::QuaternionF q) {
	require(finite(p) && finite(q), "Invalid teleport");
	if (auto* b = body(id)) {
		b->setPosition(p);
		b->setOrientation(q);
		b->calculateDerivedData();
		wake(id);
	}
}
void PhysicWorld::setVelocity(Id id, Vec3 v, Vec3 angular) {
	require(finite(v) && finite(angular), "Invalid velocity");
	if (auto* b = body(id); b && b->type == BodyType::Dynamic) {
		b->setVelocity(v);
		b->setRotation(angular);
		wake(id);
	}
}
void PhysicWorld::solveRigid(float dt) {
	auto& broad = *mPipeline.broadPhase;
	broad.reset(settings.cellSize);
	for (const auto& [id, b] : mBodies)
		broad.upsert(id, COLLISION::bounds(geometry(b)));
	std::vector<std::pair<Id, Id>> pairs;
	broad.pairs(pairs);
	PHYSICS_COMMON::canonicalPairs(pairs);
	mContactCount = 0;
	for (auto [aid, bid] : pairs) {
		if (!mBodies.contains(aid) || !mBodies.contains(bid))
			throw std::runtime_error("Broad phase returned unknown proxy");
		auto& a = mBodies.at(aid);
		auto& b = mBodies.at(bid);
		if ((a.type == BodyType::Static && b.type == BodyType::Static) || !(a.layer & b.mask) || !(b.layer & a.mask))
			continue;
		if (!COLLISION::bounds(geometry(a)).overlaps(COLLISION::bounds(geometry(b))))
			continue;
		auto manifold = detect(geometry(a), geometry(b));
		if (mContactCount + manifold.count > mContacts.size())
			throw std::runtime_error("3D contact buffer exhausted");
		for (int i = 0; i < manifold.count; ++i) {
			auto& c = mContacts[mContactCount++];
			c.contactPoint = manifold.points[i].position;
			c.penetration = manifold.points[i].penetration;
			c.contactNormal = -manifold.normal; // Legacy solver uses B-to-A.
			c.setBodyData(&a, &b, std::sqrt(a.friction * b.friction), std::max(a.restitution, b.restitution));
		}
	}
	mResolver.setIterations(settings.velocityIterations, settings.positionIterations);
	mResolver.resolveContacts(mContacts.data(), static_cast<unsigned>(mContactCount), dt);
	for (auto& [id, b] : mBodies)
		b.calculateDerivedData();
}
void PhysicWorld::step(float dt) {
	require(std::isfinite(dt) && dt > 0 && finite(settings.gravity) && std::isfinite(settings.cellSize) && settings.cellSize > 0
			&& settings.substeps > 0 && settings.substeps <= 128 && settings.velocityIterations > 0 && settings.positionIterations > 0
			&& settings.particleIterations > 0,
		"Invalid physics step/settings");
	for (auto& [id, f] : forces.fields())
		f.validate();
	// Preserve inputs submitted through the inherited RigidBody API as well.
	for (auto& [id, b] : mBodies) {
		b.force += b.getAccumulatedForce();
		b.torque += b.getAccumulatedTorque();
		b.clearAccumulators();
	}
	float h = dt / settings.substeps;
	for (int substep = 0; substep < settings.substeps; ++substep) {
		for (auto& [id, b] : mBodies)
			if (b.type == BodyType::Dynamic) {
				Vec3 generated, torque;
				bool shouldWake = false;
				for (auto& [fid, f] : forces.fields()) {
					if ((f.target && f.target != id) || !(f.targets & PHYSICS_COMMON::Rigid) || !(f.mask & b.layer))
						continue;
					auto point = b.getPointInWorldSpace(f.localPoint), r = point - b.getPosition();
					auto force = f.evaluate(point, b.getVelocity() + b.getRotation().cross(r), b.getInverseMass(), mTime);
					generated += force;
					torque += r.cross(force);
					shouldWake = shouldWake || (f.wake && force.squareMagnitude() > 1e-12f);
				}
				if (shouldWake && !b.getAwake())
					wake(id);
				b.clearAccumulators();
				if (b.getAwake()) {
					b.addForce(b.force + generated);
					b.addTorque(b.torque + torque);
				}
				b.setAcceleration(settings.gravity * b.gravityScale + b.extraAcceleration);
				b.integrate(h);
			}
		solveRigid(h);
		stepParticles(h);
		mTime += h;
	}
	for (auto& [id, b] : mBodies) {
		b.force = {};
		b.torque = {};
	}
}
void PhysicWorld::debugDraw(const DebugDraw& draw) const {
	for (auto& [id, b] : mBodies) {
		if (b.shape.kind == Shape::Kind::Sphere) {
			if (draw.sphere)
				draw.sphere(b.getPosition(), b.shape.radius, 0x50e080ff);
		}
		else if (draw.line) {
			std::array<Vec3, 8> v;
			for (int i = 0; i < 8; ++i)
				v[i] = b.getPointInWorldSpace(Vec3(i & 1 ? b.shape.halfSize.x : -b.shape.halfSize.x, i & 2 ? b.shape.halfSize.y : -b.shape.halfSize.y,
					i & 4 ? b.shape.halfSize.z : -b.shape.halfSize.z));
			for (int i = 0; i < 8; ++i)
				for (int axis = 0; axis < 3; ++axis)
					if (!(i & (1 << axis)))
						draw.line(v[i], v[i | (1 << axis)], 0x50e080ff);
		}
	}
	for (auto& [id, s] : mSoft) {
		if (draw.line)
			for (auto l : s.links)
				draw.line(s.particles[l.a].position, s.particles[l.b].position, 0xe080ffff);
		if (draw.triangle)
			for (auto t : s.triangles)
				draw.triangle(s.particles[t[0]].position, s.particles[t[1]].position, s.particles[t[2]].position, 0xe080ffff);
		if (draw.sphere)
			for (auto p : s.particles)
				draw.sphere(p.position, s.radius, 0xe080ffff);
	}
	if (draw.sphere)
		for (auto& [id, f] : mFluids)
			for (auto p : f.particles)
				draw.sphere(p.position, f.radius, 0x60b0ffff);
}
}

#include "physics2d.h"
#include "spatialGrid.h"
#include <set>
#include <stdexcept>

namespace IKIGAI::PHYSICS2D {
World::World()
	: World(Settings { }, CollisionPipeline { }) { }
World::World(Settings config, CollisionPipeline pipeline)
	: settings(config)
	, mPipeline(std::move(pipeline)) {
	mPipeline.validate();
}
bool World::detect(const Body& a, const Body& b, Contact& out) const {
	COLLISION::Manifold2D m;
	auto status = mPipeline.narrowPhase->collide({ a.shape, a.position, a.angle }, { b.shape, b.position, b.angle }, m);
	if (status == COLLISION::Status::Unsupported)
		throw std::runtime_error("Narrow phase does not support shape pair");
	out = { };
	out.a = a.id;
	out.b = b.id;
	if (status == COLLISION::Status::Separated)
		return false;
	if (m.count < 1 || m.count > 2 || !finite(m.normal) || std::abs(dot(m.normal, m.normal) - 1) > 0.001f || !std::isfinite(m.penetration)
		|| m.penetration < 0)
		throw std::runtime_error("Invalid 2D collision manifold");
	out.normal = m.normal;
	out.penetration = m.penetration;
	out.count = m.count;
	for (int i = 0; i < m.count; ++i) {
		if (!finite(m.points[i].position))
			throw std::runtime_error("Invalid contact position");
		out.points[i].position = m.points[i].position;
	}
	return true;
}

namespace {
	Vec2 pointVelocity(const Body& b, Vec2 r) {
		return b.velocity + perpendicular(r) * b.angularVelocity;
	}
	void impulse(Body& b, Vec2 p, Vec2 r) {
		b.velocity += p * b.inverseMass;
		b.angularVelocity += cross(r, p) * b.inverseInertia;
	}
	float effectiveMass(const Body& a, const Body& b, Vec2 ra, Vec2 rb, Vec2 n) {
		float ca = cross(ra, n), cb = cross(rb, n);
		return a.inverseMass + b.inverseMass + ca * ca * a.inverseInertia + cb * cb * b.inverseInertia;
	}
	bool dynamic(const Body& b) {
		return b.type == BodyType::Dynamic;
	}
	void require(bool ok, const char* message) {
		if (!ok)
			throw std::invalid_argument(message);
	}
}
Body* World::body(Id id) {
	auto i = mBodies.find(id);
	return i == mBodies.end() ? nullptr : &i->second;
}
SoftBody* World::softBody(Id id) {
	auto i = mSoft.find(id);
	return i == mSoft.end() ? nullptr : &i->second;
}
Fluid* World::fluid(Id id) {
	auto i = mFluids.find(id);
	return i == mFluids.end() ? nullptr : &i->second;
}
Id World::createBody(const BodyDef& def) {
	require(finite(def.position) && finite(def.velocity) && std::isfinite(def.angle) && std::isfinite(def.angularVelocity), "Nonfinite body state");
	require(std::isfinite(def.mass) && def.mass > 0 && std::isfinite(def.friction) && def.friction >= 0 && std::isfinite(def.restitution)
			&& def.restitution >= 0 && def.restitution <= 1 && std::isfinite(def.gravityScale) && std::isfinite(def.linearDamping)
			&& def.linearDamping >= 0 && std::isfinite(def.angularDamping) && def.angularDamping >= 0,
		"Invalid body material");
	Body b;
	static_cast<BodyDef&>(b) = def;
	b.shape = def.shape.kind == Shape::Kind::Circle ? Shape::circle(def.shape.radius) : Shape::polygon(def.shape.vertices);
	b.id = mNextId++;
	if (dynamic(b)) {
		b.inverseMass = 1 / b.mass;
		b.inverseInertia = 1 / b.shape.inertia(b.mass);
	}
	else {
		b.velocity = { };
		b.angularVelocity = 0;
	}
	mBodies.emplace(b.id, b);
	return b.id;
}
Id World::createJoint(const JointDef& def) {
	require(body(def.a) && body(def.b) && def.a != def.b, "Joint requires two existing distinct bodies");
	require(finite(def.localAnchorA) && finite(def.localAnchorB) && std::isfinite(def.distance) && def.distance >= 0, "Invalid joint");
	Joint j;
	static_cast<JointDef&>(j) = def;
	j.id = mNextId++;
	mJoints.emplace(j.id, j);
	wake(j.a);
	wake(j.b);
	return j.id;
}
void World::destroyJoint(Id id) {
	if (auto it = mJoints.find(id); it != mJoints.end()) {
		auto j = it->second;
		wake(j.a);
		wake(j.b);
		mJoints.erase(it);
	}
}
void World::destroyBody(Id id) {
	mPipeline.broadPhase->remove(id);
	forces.removeTarget(id);
	wake(id);
	std::erase_if(mJoints, [&](auto& e) { return e.second.a == id || e.second.b == id; });
	std::erase_if(mContacts, [&](const Contact& c) { return c.a == id || c.b == id; });
	mBodies.erase(id);
}
void World::clear() {
	mPipeline.broadPhase->reset(settings.cellSize);
	mBodies.clear();
	mJoints.clear();
	mSoft.clear();
	mFluids.clear();
	mContacts.clear();
	forces.clear();
	mTime = 0;
}
void World::wake(Id id) {
	std::set<Id> visited;
	std::vector<Id> pending { id };
	while (!pending.empty()) {
		Id current = pending.back();
		pending.pop_back();
		if (!visited.insert(current).second)
			continue;
		auto* b = body(current);
		if (!b)
			continue;
		if (!dynamic(*b)) {
			// Moving/removing a static support must wake its sleeping neighbours.
			// Never traverse THROUGH a static body encountered from another body.
			if (current == id) {
				for (const auto& [jid, j] : mJoints) {
					if (j.a == id)
						pending.push_back(j.b);
					if (j.b == id)
						pending.push_back(j.a);
				}
				for (const auto& c : mContacts) {
					if (c.a == id)
						pending.push_back(c.b);
					if (c.b == id)
						pending.push_back(c.a);
				}
			}
			continue;
		}
		b->sleeping = false;
		b->sleepTime = 0;
		for (const auto& [jid, j] : mJoints) {
			if (j.a == current)
				pending.push_back(j.b);
			if (j.b == current)
				pending.push_back(j.a);
		}
		for (const auto& c : mContacts) {
			if (c.a == current)
				pending.push_back(c.b);
			if (c.b == current)
				pending.push_back(c.a);
		}
	}
}
void World::applyForce(Id id, Vec2 force, Vec2 point) {
	require(finite(force) && finite(point), "Nonfinite force");
	if (auto* b = body(id); b && dynamic(*b)) {
		wake(id);
		b->force += force;
		b->torque += cross(point - b->position, force);
	}
}
void World::applyImpulse(Id id, Vec2 p, Vec2 point) {
	require(finite(p) && finite(point), "Nonfinite impulse");
	if (auto* b = body(id); b && dynamic(*b)) {
		wake(id);
		impulse(*b, p, point - b->position);
	}
}
void World::teleport(Id id, Vec2 p, float angle) {
	require(finite(p) && std::isfinite(angle), "Nonfinite transform");
	if (auto* b = body(id)) {
		wake(id);
		b->position = p;
		b->angle = angle;
	}
}
void World::setVelocity(Id id, Vec2 v, float angular) {
	require(finite(v) && std::isfinite(angular), "Nonfinite velocity");
	if (auto* b = body(id); b && dynamic(*b)) {
		wake(id);
		b->velocity = v;
		b->angularVelocity = angular;
	}
}
void World::findContacts() {
	auto& broad = *mPipeline.broadPhase;
	broad.reset(settings.cellSize);
	for (const auto& [id, b] : mBodies) {
		auto box = b.bounds();
		broad.upsert(id, UTILS::SPATIAL::SpatialQuery<2, Vec2>::bounds(box.min, box.max));
	}
	std::vector<std::pair<Id, Id>> pairs;
	broad.pairs(pairs);
	PHYSICS_COMMON::canonicalPairs(pairs);
	std::set<std::pair<Id, Id>> exceptions;
	for (const auto& [id, j] : mJoints)
		if (!j.collideConnected)
			exceptions.emplace(std::min(j.a, j.b), std::max(j.a, j.b));
	mContacts.clear();
	for (auto [aid, bid] : pairs) {
		if (!mBodies.contains(aid) || !mBodies.contains(bid))
			throw std::runtime_error("Broad phase returned unknown proxy");
		auto& a = mBodies.at(aid);
		auto& b = mBodies.at(bid);
		if ((!dynamic(a) && !dynamic(b)) || !(a.mask & b.layer) || !(b.mask & a.layer) || exceptions.contains({ aid, bid })
			|| !a.bounds().overlaps(b.bounds()))
			continue;
		Contact c;
		if (!detect(a, b, c))
			continue;
		if (a.sleeping && dynamic(b) && !b.sleeping
			&& (length(b.velocity) > settings.sleepLinearSpeed || std::abs(b.angularVelocity) > settings.sleepAngularSpeed))
			wake(a.id);
		if (b.sleeping && dynamic(a) && !a.sleeping
			&& (length(a.velocity) > settings.sleepLinearSpeed || std::abs(a.angularVelocity) > settings.sleepAngularSpeed))
			wake(b.id);
		for (int i = 0; i < c.count; ++i) {
			auto p = c.points[i].position;
			float speed = dot(pointVelocity(b, p - b.position) - pointVelocity(a, p - a.position), c.normal);
			// Do not bounce at resting-contact speeds: gravity would otherwise
			// generate a tiny bounce on every step and prevent sleeping.
			c.points[i].bounce = speed < -1 ? -std::max(a.restitution, b.restitution) * speed : 0;
		}
		mContacts.push_back(c);
	}
}
void World::solveContacts() {
	for (auto& c : mContacts) {
		auto& a = mBodies.at(c.a);
		auto& b = mBodies.at(c.b);
		if ((a.sleeping || !dynamic(a)) && (b.sleeping || !dynamic(b)))
			continue;
		if (a.sleeping)
			wake(a.id);
		if (b.sleeping)
			wake(b.id);
		Vec2 tangent = perpendicular(c.normal);
		float friction = std::sqrt(a.friction * b.friction);
		for (int i = 0; i < c.count; ++i) {
			auto& p = c.points[i];
			Vec2 ra = p.position - a.position, rb = p.position - b.position;
			Vec2 relative = pointVelocity(b, rb) - pointVelocity(a, ra);
			float k = effectiveMass(a, b, ra, rb, c.normal);
			if (k < Epsilon)
				continue;
			// Импульс j гасит относительную скорость вдоль нормали контакта.
			// В знаменателе — обратные массы и вклад плеча рычага. Поэтому
			// удар мимо центра масс одновременно меняет скорость и вращение.
			float old = p.normalImpulse;
			p.normalImpulse = std::max(0.0f, old + (p.bounce - dot(relative, c.normal)) / k);
			Vec2 push = c.normal * (p.normalImpulse - old);
			impulse(a, -push, ra);
			impulse(b, push, rb);
			relative = pointVelocity(b, rb) - pointVelocity(a, ra);
			k = effectiveMass(a, b, ra, rb, tangent);
			if (k < Epsilon)
				continue;
			old = p.tangentImpulse;
			// Трение Кулона: касательный импульс ограничен mu * нормальный.
			float limit = friction * p.normalImpulse;
			p.tangentImpulse = std::clamp(old - dot(relative, tangent) / k, -limit, limit);
			push = tangent * (p.tangentImpulse - old);
			impulse(a, -push, ra);
			impulse(b, push, rb);
		}
	}
}
void World::solveJoints(float dt, bool position) {
	for (auto& [id, j] : mJoints) {
		auto& a = mBodies.at(j.a);
		auto& b = mBodies.at(j.b);
		if ((a.sleeping || !dynamic(a)) && (b.sleeping || !dynamic(b)))
			continue;
		if (a.sleeping)
			wake(a.id);
		if (b.sleeping)
			wake(b.id);
		Vec2 ra = rotate(j.localAnchorA, a.angle), rb = rotate(j.localAnchorB, b.angle);
		Vec2 delta = b.position + rb - a.position - ra, push;
		Vec2 relative = pointVelocity(b, rb) - pointVelocity(a, ra);
		if (j.kind == JointDef::Kind::Distance) {
			float d = length(delta);
			Vec2 n = normalized(delta);
			float k = effectiveMass(a, b, ra, rb, n);
			if (k < Epsilon)
				continue;
			float error = std::clamp(d - j.distance, -0.2f, 0.2f);
			float lambda = position ? -0.5f * error / k : -(dot(relative, n) + 0.1f * error / dt) / k;
			push = n * lambda;
		}
		else {
			// A hinge constrains X and Y together. Solve the 2x2 effective mass
			// matrix instead of alternating axes, so rotated anchors behave alike.
			float inv = a.inverseMass + b.inverseMass;
			float xx = inv + ra.y * ra.y * a.inverseInertia + rb.y * rb.y * b.inverseInertia;
			float yy = inv + ra.x * ra.x * a.inverseInertia + rb.x * rb.x * b.inverseInertia;
			float xy = -ra.x * ra.y * a.inverseInertia - rb.x * rb.y * b.inverseInertia;
			float det = xx * yy - xy * xy;
			if (det < Epsilon)
				continue;
			Vec2 rhs = position ? -delta * 0.5f : -(relative + delta * (0.1f / dt));
			push = { (yy * rhs.x - xy * rhs.y) / det, (-xy * rhs.x + xx * rhs.y) / det };
		}
		if (position) {
			a.position -= push * a.inverseMass;
			b.position += push * b.inverseMass;
			a.angle -= cross(ra, push) * a.inverseInertia;
			b.angle += cross(rb, push) * b.inverseInertia;
		}
		else {
			impulse(a, -push, ra);
			impulse(b, push, rb);
		}
	}
}
void World::updateSleeping(float dt) {
	// Dynamic contact/joint components form islands. Static ground is a boundary,
	// not a graph vertex: otherwise one floor joins every object into one island.
	std::map<Id, std::vector<Id>> graph;
	auto connect = [&](Id a, Id b) {
		if (dynamic(mBodies.at(a)) && dynamic(mBodies.at(b))) {
			graph[a].push_back(b);
			graph[b].push_back(a);
		}
	};
	for (const auto& c : mContacts)
		connect(c.a, c.b);
	for (const auto& [id, j] : mJoints)
		connect(j.a, j.b);
	std::set<Id> seen;
	for (auto& [id, b] : mBodies) {
		if (!dynamic(b) || seen.contains(id))
			continue;
		std::vector<Id> island, pending { id };
		bool quiet = settings.sleeping;
		while (!pending.empty()) {
			Id current = pending.back();
			pending.pop_back();
			if (!seen.insert(current).second)
				continue;
			island.push_back(current);
			auto& member = mBodies.at(current);
			quiet = quiet && member.allowSleep && length(member.velocity) < settings.sleepLinearSpeed
				&& std::abs(member.angularVelocity) < settings.sleepAngularSpeed;
			for (Id n : graph[current])
				pending.push_back(n);
		}
		bool ready = quiet;
		for (Id i : island) {
			auto& member = mBodies.at(i);
			member.sleepTime = quiet ? member.sleepTime + dt : 0;
			ready = ready && member.sleepTime >= settings.sleepDelay;
		}
		for (Id i : island) {
			auto& member = mBodies.at(i);
			member.sleeping = ready;
			if (ready) {
				member.velocity = { };
				member.angularVelocity = 0;
			}
		}
	}
}
void World::step(float dt) {
	require(std::isfinite(dt) && dt > 0, "Physics dt must be positive");
	require(finite(settings.gravity) && std::isfinite(settings.cellSize) && settings.cellSize > 0 && settings.substeps > 0 && settings.substeps <= 128
			&& settings.velocityIterations > 0 && settings.positionIterations > 0 && settings.particleIterations > 0
			&& std::isfinite(settings.contactSlop) && settings.contactSlop >= 0 && settings.sleepLinearSpeed >= 0 && settings.sleepAngularSpeed >= 0
			&& settings.sleepDelay >= 0,
		"Invalid physics settings");
	const float h = dt / settings.substeps;
	for (auto& [id, f] : forces.fields())
		f.validate();
	for (int step = 0; step < settings.substeps; ++step) {
		for (auto& [id, b] : mBodies) {
			Vec2 generated { };
			float torque = 0;
			bool shouldWake = false;
			if (dynamic(b))
				for (auto& [fid, f] : forces.fields()) {
					if ((f.target && f.target != id) || !(f.targets & PHYSICS_COMMON::Rigid) || !(f.mask & b.layer))
						continue;
					auto r = rotate(f.localPoint, b.angle);
					auto force = f.evaluate(b.position + r, b.velocity + perpendicular(r) * b.angularVelocity, b.inverseMass, mTime);
					generated += force;
					torque += cross(r, force);
					shouldWake = shouldWake || (f.wake && dot(force, force) > 1e-12f);
				}
			if (shouldWake && b.sleeping)
				wake(id);
			if (dynamic(b) && !b.sleeping) {
				// Полунеявный Euler: сначала v += a*h, затем x += v*h.
				// Силы действуют ВСЕ подшаги. Их очистка здесь ослабила бы силу
				// в settings.substeps раз, поэтому очищаем их после полного шага.
				b.velocity += (settings.gravity * b.gravityScale + (b.force + generated) * b.inverseMass) * h;
				b.angularVelocity += (b.torque + torque) * b.inverseInertia * h;
				b.velocity *= 1 / (1 + b.linearDamping * h);
				b.angularVelocity /= 1 + b.angularDamping * h;
				b.position += b.velocity * h;
				b.angle += b.angularVelocity * h;
			}
		}
		findContacts();
		for (int i = 0; i < settings.velocityIterations; ++i) {
			solveJoints(h, false);
			solveContacts();
		}
		for (int i = 0; i < settings.positionIterations; ++i) {
			for (auto& c : mContacts) {
				auto& a = mBodies.at(c.a);
				auto& b = mBodies.at(c.b);
				if ((a.sleeping || !dynamic(a)) && (b.sleeping || !dynamic(b)))
					continue;
				Contact fresh;
				if (!detect(a, b, fresh))
					continue;
				float inv = a.inverseMass + b.inverseMass;
				if (inv < Epsilon)
					continue;
				Vec2 correction = fresh.normal * (0.5f * std::max(0.0f, fresh.penetration - settings.contactSlop) / inv);
				a.position -= correction * a.inverseMass;
				b.position += correction * b.inverseMass;
			}
			solveJoints(h, true);
		}
		stepParticles(h);
		updateSleeping(h);
		mTime += h;
	}
	for (auto& [id, b] : mBodies) {
		b.force = { };
		b.torque = 0;
	}
}
void World::debugDraw(const DebugDraw& draw) const {
	for (const auto& [id, b] : mBodies) {
		auto colour = !dynamic(b) ? 0x909090ffu : b.sleeping ? 0x70a0ffffu : 0x50e080ffu;
		if (b.shape.kind == Shape::Kind::Circle) {
			if (draw.circle)
				draw.circle(b.position, b.shape.radius, colour);
		}
		else if (draw.line)
			for (std::size_t i = 0; i < b.shape.vertices.size(); ++i)
				draw.line(b.position + rotate(b.shape.vertices[i], b.angle),
					b.position + rotate(b.shape.vertices[(i + 1) % b.shape.vertices.size()], b.angle), colour);
	}
	if (draw.line) {
		for (const auto& c : mContacts)
			for (int i = 0; i < c.count; ++i)
				draw.line(c.points[i].position, c.points[i].position + c.normal * 0.25f, 0xff6060ff);
		for (const auto& [id, j] : mJoints)
			draw.line(mBodies.at(j.a).position + rotate(j.localAnchorA, mBodies.at(j.a).angle),
				mBodies.at(j.b).position + rotate(j.localAnchorB, mBodies.at(j.b).angle), 0xffff60ff);
		for (const auto& [id, s] : mSoft)
			for (auto link : s.links)
				draw.line(s.particles[link.a].position, s.particles[link.b].position, 0xe080ffff);
	}
	if (draw.circle) {
		for (const auto& [id, s] : mSoft)
			for (const auto& p : s.particles)
				draw.circle(p.position, s.radius, 0xe080ffff);
		for (const auto& [id, f] : mFluids)
			for (const auto& p : f.particles)
				draw.circle(p.position, f.radius, 0x60b0ffff);
	}
}
}

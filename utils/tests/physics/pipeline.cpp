#include "physics2dModule/physics2d.h"
#include "physicsModule/physicWorld.h"
#include <iostream>
#include <random>
#include <stdexcept>
using namespace IKIGAI;
void check(bool ok, const char* message) {
	if (!ok)
		throw std::runtime_error(message);
}
struct Spy2D : COLLISION::DefaultNarrowPhase2D {
	mutable int calls = 0;
	COLLISION::Status collide(const COLLISION::ShapeView2D& a, const COLLISION::ShapeView2D& b, COLLISION::Manifold2D& out) const override {
		++calls;
		return DefaultNarrowPhase2D::collide(a, b, out);
	}
};
struct Spy3D : COLLISION::DefaultNarrowPhase3D {
	mutable int calls = 0;
	COLLISION::Status collide(const COLLISION::ShapeView3D& a, const COLLISION::ShapeView3D& b, COLLISION::Manifold3D& out) const override {
		++calls;
		return DefaultNarrowPhase3D::collide(a, b, out);
	}
};
template <std::size_t N> struct NeighborSpy : UTILS::SPATIAL::INeighborSearchFactory<N> {
	mutable int builds = 0;
	std::unique_ptr<UTILS::SPATIAL::ISpatialIndex<N>> create(float size) const override {
		++builds;
		return std::make_unique<UTILS::SPATIAL::GridIndex<N>>(size);
	}
};
void injection2d() {
	using namespace PHYSICS2D;
	CollisionPipeline pipeline;
	auto spy = std::make_unique<Spy2D>();
	auto* observed = spy.get();
	pipeline.narrowPhase = std::move(spy);
	pipeline.broadPhase = std::make_unique<BvhBroadPhase2D>();
	auto neighbors = std::make_unique<NeighborSpy<2>>();
	auto* neighborCalls = neighbors.get();
	pipeline.neighbors = std::move(neighbors);
	Settings settings;
	settings.gravity = {};
	settings.substeps = 1;
	World world(settings, std::move(pipeline));
	BodyDef floor;
	floor.type = BodyType::Static;
	floor.shape = Shape::box({ 2, .5f });
	world.createBody(floor);
	BodyDef ball;
	ball.position = { 0, .8f };
	ball.shape = Shape::circle(.5f);
	auto id = world.createBody(ball);
	world.step(.01f);
	check(observed->calls >= 1 + settings.positionIterations, "Position solver bypassed injected narrow phase");
	world.destroyBody(id);
	int before = observed->calls;
	SoftBodyDef soft;
	soft.radius = .1f;
	soft.particles = { { { 0, .55f }, {}, 1 } };
	auto sid = world.createSoftBody(soft);
	world.step(.01f);
	check(observed->calls > before, "2D particle-rigid bypassed narrow phase");
	check(neighborCalls->builds > 0, "2D particles bypassed neighbor factory");
	check(world.softBody(sid)->particles[0].position.y > .59f, "2D particle contact response lost");
	world.clear();
	world.step(.01f);
	check(world.contacts().empty(), "2D clear retained contacts");
}
void injection3d() {
	using namespace PHYSICS;
	CollisionPipeline pipeline;
	auto spy = std::make_unique<Spy3D>();
	auto* observed = spy.get();
	pipeline.narrowPhase = std::move(spy);
	pipeline.broadPhase = std::make_unique<BvhBroadPhase3D>();
	auto neighbors = std::make_unique<NeighborSpy<3>>();
	auto* neighborCalls = neighbors.get();
	pipeline.neighbors = std::move(neighbors);
	Settings settings;
	settings.gravity = {};
	settings.substeps = 1;
	World world(settings, std::move(pipeline));
	BodyDef floor;
	floor.type = BodyType::Static;
	floor.shape = Shape::box({ 2, .5f, 2 });
	world.createBody(floor);
	SoftBodyDef soft;
	soft.radius = .1f;
	soft.particles = { { { 0, .55f, 0 }, {}, 1 } };
	auto sid = world.createSoftBody(soft);
	world.step(.01f);
	check(observed->calls > 0, "3D particle-rigid bypassed narrow phase");
	check(world.softBody(sid)->particles[0].position.y > .59f, "3D particle contact response lost");
	FluidDef fluid;
	fluid.particles = { { { 1, 1, 0 }, {}, 1 }, { { 1.05f, 1, 0 }, {}, 1 } };
	world.createFluid(fluid);
	world.step(.01f);
	check(neighborCalls->builds > 0, "3D liquid bypassed neighbor factory");
	int before = observed->calls;
	BodyDef ball;
	ball.position = { 0, .8f, 0 };
	ball.shape = Shape::sphere(.5f);
	world.createBody(ball);
	world.step(.01f);
	check(observed->calls > before && !world.contacts().empty(), "3D rigid detector bypassed custom strategy");
}
void legacyComparison() {
	using namespace PHYSICS;
	World world;
	BodyDef d;
	auto aid = world.createBody(d), bid = world.createBody(d);
	auto& a = *world.body(aid);
	auto& b = *world.body(bid);
	std::mt19937 rng(9);
	std::uniform_real_distribution<float> position(-1.1f, 1.1f), angle(-1, 1);
	int overlaps = 0;
	for (int kindA = 0; kindA < 2; ++kindA)
		for (int kindB = 0; kindB < 2; ++kindB)
			for (int sample = 0; sample < 300; ++sample) {
				a.shape = kindA ? Shape::box({ .5f, .6f, .7f }) : Shape::sphere(.6f);
				b.shape = kindB ? Shape::box({ .7f, .5f, .6f }) : Shape::sphere(.7f);
				a.setPosition({ position(rng), position(rng), position(rng) });
				b.setPosition({ position(rng), position(rng), position(rng) });
				a.setOrientation(MATH::QuaternionF(MATH::Vector3f { angle(rng), angle(rng), angle(rng) }));
				b.setOrientation(MATH::QuaternionF(MATH::Vector3f { angle(rng), angle(rng), angle(rng) }));
				a.calculateDerivedData();
				b.calculateDerivedData();
				Contact contact {};
				CollisionData data {};
				data.contactArray = &contact;
				data.reset(1);
				data.friction = .5f;
				data.restitution = .1f;
				CollisionSphere sa, sb;
				sa.body = &a;
				sb.body = &b;
				sa.radius = a.shape.radius;
				sb.radius = b.shape.radius;
				sa.calculateInternals();
				sb.calculateInternals();
				CollisionBox ba, bb;
				ba.body = &a;
				bb.body = &b;
				ba.halfSize = a.shape.halfSize;
				bb.halfSize = b.shape.halfSize;
				ba.calculateInternals();
				bb.calculateInternals();
				if (!kindA && !kindB)
					CollisionDetector::sphereAndSphere(sa, sb, &data);
				else if (kindA && kindB)
					CollisionDetector::boxAndBox(ba, bb, &data);
				else if (kindA)
					CollisionDetector::boxAndSphere(ba, sb, &data);
				else
					CollisionDetector::boxAndSphere(bb, sa, &data);
				COLLISION::Manifold3D m;
				auto status = COLLISION::DefaultNarrowPhase3D {}.collide(geometry(a), geometry(b), m);
				check((status == COLLISION::Status::Contact) == (data.contactCount != 0), "3D geometry changed overlap relative to legacy");
				if (!data.contactCount)
					continue;
				++overlaps;
				auto normal = contact.body[0] == &a ? -contact.contactNormal : contact.contactNormal;
				check((m.normal - normal).magnitude() < .002f, "3D manifold normal differs from legacy");
				check(std::abs(m.points[0].penetration - contact.penetration) < .002f, "3D penetration differs from legacy");
				check((m.points[0].position - contact.contactPoint).magnitude() < .003f, "3D contact point differs from legacy");
			}
	check(overlaps > 100, "Randomized comparison did not exercise contacts");
}
void contracts() {
	COLLISION::DefaultNarrowPhase2D n;
	auto shape = COLLISION::Shape2D::circle(1);
	COLLISION::Manifold2D out;
	n.setHandler(shape.kind, shape.kind, [](const auto&, const auto&, auto&) { return COLLISION::Status::Unsupported; });
	PHYSICS2D::CollisionPipeline pipeline;
	pipeline.narrowPhase = std::make_unique<COLLISION::DefaultNarrowPhase2D>(std::move(n));
	PHYSICS2D::World w(std::move(pipeline));
	PHYSICS2D::BodyDef d;
	w.createBody(d);
	w.createBody(d);
	bool rejected = false;
	try {
		w.step(.01f);
	} catch (const std::runtime_error&) {
		rejected = true;
	}
	check(rejected, "Unsupported shape pair silently treated as separated");
	PHYSICS::CollisionPipeline empty;
	empty.neighbors.reset();
	rejected = false;
	try {
		PHYSICS::World bad(std::move(empty));
	} catch (const std::invalid_argument&) {
		rejected = true;
	}
	check(rejected, "Null strategy accepted");
	PHYSICS::World limited(PHYSICS::Settings {}, PHYSICS::CollisionPipeline {}, 1);
	PHYSICS::BodyDef body;
	for (int i = 0; i < 3; ++i)
		limited.createBody(body);
	rejected = false;
	try {
		limited.step(.01f);
	} catch (const std::runtime_error& e) {
		rejected = std::string(e.what()).find("buffer exhausted") != std::string::npos;
	}
	check(rejected, "Contact capacity silently truncated manifolds");
	PHYSICS::CollisionPipeline malformed;
	auto invalid = std::make_unique<PHYSICS::DefaultNarrowPhase3D>();
	invalid->setHandler(PHYSICS::Shape::Kind::Sphere, PHYSICS::Shape::Kind::Sphere, [](const auto&, const auto&, auto& out) {
		out.count = 99;
		return COLLISION::Status::Contact;
	});
	malformed.narrowPhase = std::move(invalid);
	PHYSICS::World bad(std::move(malformed));
	bad.createBody(body);
	bad.createBody(body);
	rejected = false;
	try {
		bad.step(.01f);
	} catch (const std::runtime_error&) {
		rejected = true;
	}
	check(rejected, "Malformed custom manifold accepted");
}
template <std::size_t N> struct DuplicatePairs : UTILS::SPATIAL::ISpatialIndex<N> {
	using Bounds = MATH::GEOMETRY::Aabb<N>;
	UTILS::SPATIAL::BruteForceIndex<N> index;
	void reset(float s) override {
		index.reset(s);
	}
	void upsert(std::uint64_t id, const Bounds& b) override {
		index.upsert(id, b);
	}
	bool remove(std::uint64_t id) override {
		return index.remove(id);
	}
	void query(const Bounds& b, std::vector<std::uint64_t>& out) const override {
		index.query(b, out);
	}
	void pairs(std::vector<std::pair<std::uint64_t, std::uint64_t>>& out) const override {
		index.pairs(out);
		auto original = out;
		for (auto [a, b] : original) {
			out.emplace_back(b, a);
			out.emplace_back(a, b);
			out.emplace_back(a, a);
		}
	}
};
void canonicalization() {
	PHYSICS2D::CollisionPipeline p;
	p.broadPhase = std::make_unique<DuplicatePairs<2>>();
	PHYSICS2D::World w(std::move(p));
	w.settings.substeps = 1;
	PHYSICS2D::BodyDef d;
	d.position = { 0, 0 };
	w.createBody(d);
	d.position = { .8f, 0 };
	w.createBody(d);
	w.step(.01f);
	check(w.contacts().size() == 1, "Duplicate broad-phase pairs reached 2D solver");
	PHYSICS::CollisionPipeline p3;
	p3.broadPhase = std::make_unique<DuplicatePairs<3>>();
	PHYSICS::World w3(std::move(p3));
	w3.settings.substeps = 1;
	PHYSICS::BodyDef d3;
	w3.createBody(d3);
	d3.position = { .8f, 0, 0 };
	w3.createBody(d3);
	w3.step(.01f);
	check(w3.contacts().size() == 1, "Duplicate broad-phase pairs reached 3D solver");
}
void backendScenarios() {
	for (int backend = 0; backend < 3; ++backend) {
		PHYSICS2D::CollisionPipeline p2;
		PHYSICS::CollisionPipeline p3;
		if (backend == 1) {
			p2.broadPhase = std::make_unique<PHYSICS2D::BvhBroadPhase2D>();
			p2.neighbors = std::make_unique<PHYSICS2D::BvhNeighborSearchFactory2D>();
			p3.broadPhase = std::make_unique<PHYSICS::BvhBroadPhase3D>();
			p3.neighbors = std::make_unique<PHYSICS::BvhNeighborSearchFactory3D>();
		}
		else if (backend == 2) {
			p2.broadPhase = std::make_unique<PHYSICS2D::BruteForceBroadPhase2D>();
			p2.neighbors = std::make_unique<UTILS::SPATIAL::NeighborSearchFactory<2, PHYSICS2D::BruteForceBroadPhase2D>>();
			p3.broadPhase = std::make_unique<PHYSICS::BruteForceBroadPhase3D>();
			p3.neighbors = std::make_unique<UTILS::SPATIAL::NeighborSearchFactory<3, PHYSICS::BruteForceBroadPhase3D>>();
		}
		PHYSICS2D::World w2(std::move(p2));
		PHYSICS::World w3(std::move(p3));
		PHYSICS2D::BodyDef floor2;
		floor2.type = PHYSICS2D::BodyType::Static;
		floor2.shape = PHYSICS2D::Shape::box({ 10, .5f });
		floor2.position = { 0, -.5f };
		w2.createBody(floor2);
		PHYSICS::BodyDef floor3;
		floor3.type = PHYSICS::BodyType::Static;
		floor3.shape = PHYSICS::Shape::box({ 10, .5f, 10 });
		floor3.position = { 0, -.5f, 0 };
		w3.createBody(floor3);
		PHYSICS2D::BodyDef ball2;
		ball2.position = { -2, 1 };
		auto b2 = w2.createBody(ball2);
		PHYSICS::BodyDef ball3;
		ball3.position = { -2, 1, 0 };
		auto b3 = w3.createBody(ball3);
		PHYSICS2D::ClothDef cloth2;
		cloth2.width = cloth2.height = 3;
		cloth2.position = { 2, 2 };
		auto c2 = w2.createCloth(cloth2);
		PHYSICS::ClothDef cloth3;
		cloth3.width = cloth3.height = 3;
		cloth3.position = { 2, 2, 0 };
		auto c3 = w3.createCloth(cloth3);
		PHYSICS2D::FluidDef fluid2;
		fluid2.particles = { { { 0, .08f }, {}, 1 }, { { .08f, .08f }, {}, 1 } };
		auto f2 = w2.createFluid(fluid2);
		PHYSICS::FluidDef fluid3;
		fluid3.particles = { { { 0, .08f, 0 }, {}, 1 }, { { .08f, .08f, 0 }, {}, 1 } };
		auto f3 = w3.createFluid(fluid3);
		for (int step = 0; step < 120; ++step) {
			w2.step(1.f / 120);
			w3.step(1.f / 120);
		}
		check(std::abs(w2.body(b2)->position.y - .5f) < .04f, "2D backend lost floor contact");
		check(std::abs(w3.body(b3)->getPosition().y - .5f) < .04f, "3D backend lost floor contact");
		check(w2.cloth(c2)->particles[0].position.y == 2 && w3.cloth(c3)->particles[0].position.y == 2, "Backend moved pinned cloth");
		for (auto particle : w2.fluid(f2)->particles)
			check(particle.position.y >= .049f, "2D backend leaked liquid through floor");
		for (auto particle : w3.fluid(f3)->particles)
			check(particle.position.y >= .039f, "3D backend leaked liquid through floor");
	}
}
int main() {
	try {
		injection2d();
		injection3d();
		legacyComparison();
		contracts();
		canonicalization();
		backendScenarios();
		std::cout << "Pipeline injection and legacy geometry comparison passed\n";
		return 0;
	} catch (const std::exception& e) {
		std::cerr << e.what() << '\n';
		return 1;
	}
}

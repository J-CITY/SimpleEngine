#include "physics2dModule/physics2d.h"
#include "physicsModule/physicWorld.h"
#include <chrono>
#include <iomanip>
#include <iostream>

using namespace IKIGAI;
template <std::size_t N> class MeasuredIndex : public UTILS::SPATIAL::ISpatialIndex<N> {
	std::unique_ptr<UTILS::SPATIAL::ISpatialIndex<N>> mIndex;

public:
	using Bounds = MATH::GEOMETRY::Aabb<N>;
	mutable std::size_t candidates = 0;
	explicit MeasuredIndex(int kind) {
		if (kind == 0)
			mIndex = std::make_unique<UTILS::SPATIAL::GridIndex<N>>();
		else if (kind == 1)
			mIndex = std::make_unique<UTILS::SPATIAL::BvhIndex<N>>();
		else
			mIndex = std::make_unique<UTILS::SPATIAL::BruteForceIndex<N>>();
	}
	void reset(float size) override {
		mIndex->reset(size);
	}
	void upsert(std::uint64_t id, const Bounds& b) override {
		mIndex->upsert(id, b);
	}
	bool remove(std::uint64_t id) override {
		return mIndex->remove(id);
	}
	void query(const Bounds& b, std::vector<std::uint64_t>& out) const override {
		mIndex->query(b, out);
	}
	void pairs(std::vector<std::pair<std::uint64_t, std::uint64_t>>& out) const override {
		auto before = out.size();
		mIndex->pairs(out);
		candidates += out.size() - before;
	}
};
template <class World> void measure(World& world, const char* dimension, int kind, const char* scene, auto& index) {
	for (int i = 0; i < 3; ++i)
		world.step(1.f / 120);
	index.candidates = 0;
	std::size_t contacts = 0;
	std::vector<double> times;
	for (int i = 0; i < 30; ++i) {
		auto start = std::chrono::steady_clock::now();
		world.step(1.f / 120);
		times.push_back(std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count());
		contacts += world.contacts().size();
	}
	std::sort(times.begin(), times.end());
	std::cout << dimension << ',' << scene << ',' << kind << ',' << times[times.size() / 2] << ',' << times[28] << ',' << index.candidates / 30.0
			  << ',' << contacts / 30.0 << '\n';
}
void run2d(int kind, int scenario) {
	using namespace PHYSICS2D;
	CollisionPipeline p;
	auto broad = std::make_unique<MeasuredIndex<2>>(kind);
	auto* index = broad.get();
	p.broadPhase = std::move(broad);
	if (kind == 1)
		p.neighbors = std::make_unique<BvhNeighborSearchFactory2D>();
	if (kind == 2)
		p.neighbors = std::make_unique<UTILS::SPATIAL::NeighborSearchFactory<2, BruteForceBroadPhase2D>>();
	Settings settings;
	settings.substeps = 1;
	settings.particleIterations = 3;
	settings.sleeping = false;
	World w(settings, std::move(p));
	BodyDef floor;
	floor.type = BodyType::Static;
	floor.shape = Shape::box({ 20, .5f });
	floor.position = { 0, -.5f };
	w.createBody(floor);
	if (scenario < 2)
		for (int i = 0; i < 32; ++i) {
			BodyDef b;
			b.shape = Shape::box({ .1f + .02f * (scenario ? i % 8 : 0), .1f });
			b.position = { float(i % 8) * .3f, float(i / 8) * .3f + .1f };
			w.createBody(b);
		}
	if (scenario == 2) {
		ClothDef d;
		d.width = d.height = 8;
		d.position = { 0, 2 };
		w.createCloth(d);
	}
	if (scenario == 3) {
		FluidDef d;
		for (int y = 0; y < 8; ++y)
			for (int x = 0; x < 8; ++x)
				d.particles.push_back({ { x * .08f, y * .08f + .1f }, {}, 1 });
		w.createFluid(d);
	}
	const char* names[] { "rigid", "mixed", "cloth", "water" };
	measure(w, "2d", kind, names[scenario], *index);
}
void run3d(int kind, int scenario) {
	using namespace PHYSICS;
	CollisionPipeline p;
	auto broad = std::make_unique<MeasuredIndex<3>>(kind);
	auto* index = broad.get();
	p.broadPhase = std::move(broad);
	if (kind == 1)
		p.neighbors = std::make_unique<BvhNeighborSearchFactory3D>();
	if (kind == 2)
		p.neighbors = std::make_unique<UTILS::SPATIAL::NeighborSearchFactory<3, BruteForceBroadPhase3D>>();
	Settings settings;
	settings.substeps = 1;
	settings.particleIterations = 3;
	World w(settings, std::move(p));
	BodyDef floor;
	floor.type = BodyType::Static;
	floor.shape = Shape::box({ 20, .5f, 20 });
	floor.position = { 0, -.5f, 0 };
	w.createBody(floor);
	if (scenario < 2)
		for (int i = 0; i < 32; ++i) {
			BodyDef b;
			b.shape = Shape::box({ .1f + .02f * (scenario ? i % 8 : 0), .1f, .1f });
			b.position = { float(i % 8) * .3f, float(i / 8) * .3f + .1f, 0 };
			w.createBody(b);
		}
	if (scenario == 2) {
		ClothDef d;
		d.width = d.height = 8;
		d.position = { 0, 2, 0 };
		w.createCloth(d);
	}
	if (scenario == 3) {
		FluidDef d;
		for (int z = 0; z < 4; ++z)
			for (int y = 0; y < 4; ++y)
				for (int x = 0; x < 4; ++x)
					d.particles.push_back({ { x * .08f, y * .08f + .1f, z * .08f }, {}, 1 });
		w.createFluid(d);
	}
	const char* names[] { "rigid", "mixed", "cloth", "water" };
	measure(w, "3d", kind, names[scenario], *index);
}
int main() {
	std::cout << "dimension,scene,backend_0_grid_1_bvh_2_brute,median_step_ms,p95_step_ms,mean_rigid_candidates,mean_rigid_contacts\n"
			  << std::fixed << std::setprecision(4);
	for (int kind = 0; kind < 3; ++kind)
		for (int scene = 0; scene < 4; ++scene) {
			run2d(kind, scene);
			run3d(kind, scene);
		}
}

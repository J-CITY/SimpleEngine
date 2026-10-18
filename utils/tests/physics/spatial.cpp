#include "utilsModule/spatial/spatialIndex.h"
#include <chrono>
#include <iostream>
#include <random>
#include <set>
#include <stdexcept>

using namespace IKIGAI;
namespace S = UTILS::SPATIAL;
void check(bool v, const char* message) {
	if (!v)
		throw std::runtime_error(message);
}
template <std::size_t N> void spatial() {
	using Box = MATH::GEOMETRY::Aabb<N>;
	using Id = std::uint64_t;
	std::mt19937 random(1789);
	std::uniform_real_distribution<float> position(-20, 20), extent(0.01f, 3);
	std::vector<Box> boxes(180);
	for (auto& b : boxes)
		for (std::size_t axis = 0; axis < N; ++axis) {
			b.min[axis] = position(random);
			b.max[axis] = b.min[axis] + extent(random);
		}
	for (std::size_t axis = 0; axis < N; ++axis) {
		boxes[0].min[axis] = -1e6f;
		boxes[0].max[axis] = 1e6f;
	}
	// Touching, tiny negative coordinates and ID zero are all valid.
	boxes[1].min.fill(-0.1f);
	boxes[1].max.fill(0);
	boxes[2].min.fill(0);
	boxes[2].max.fill(0.1f);
	for (int algorithm = 0; algorithm < 3; ++algorithm) {
		std::unique_ptr<S::ISpatialIndex<N>> index;
		if (algorithm == 0)
			index = std::make_unique<S::GridIndex<N>>(.7f);
		if (algorithm == 1)
			index = std::make_unique<S::BvhIndex<N>>();
		if (algorithm == 2)
			index = std::make_unique<S::BruteForceIndex<N>>();
		const auto start = std::chrono::steady_clock::now();
		for (Id id = 0; id < boxes.size(); ++id)
			index->upsert(id, boxes[id]);
		std::vector<std::pair<Id, Id>> pairs;
		index->pairs(pairs);
		std::set<std::pair<Id, Id>> candidates(pairs.begin(), pairs.end());
		for (Id a = 0; a < boxes.size(); ++a)
			for (Id b = a + 1; b < boxes.size(); ++b)
				if (boxes[a].overlaps(boxes[b]))
					check(candidates.contains({ a, b }), "Broad phase missed overlapping bounds");
		for (const auto& box : boxes) {
			std::vector<Id> out;
			index->query(box, out);
			std::set<Id> found(out.begin(), out.end());
			for (Id id = 0; id < boxes.size(); ++id)
				if (box.overlaps(boxes[id]))
					check(found.contains(id), "Spatial query missed overlap");
		}
		Box moved;
		moved.min.fill(100);
		moved.max.fill(101);
		index->upsert(2, moved);
		index->remove(0);
		index->remove(1);
		std::vector<Id> out;
		index->query(moved, out);
		check(std::find(out.begin(), out.end(), 2) != out.end(), "Updated proxy missing");
		check(std::find(out.begin(), out.end(), 0) == out.end(), "Removed giant proxy survived");
		auto invalid = moved;
		invalid.max[0] = invalid.min[0] - 1;
		bool rejected = false;
		try {
			index->upsert(999, invalid);
		} catch (const std::invalid_argument&) {
			rejected = true;
		}
		check(rejected, "Accepted inverted bounds");
		invalid = moved;
		invalid.min[0] = std::numeric_limits<float>::quiet_NaN();
		rejected = false;
		try {
			index->query(invalid, out);
		} catch (const std::invalid_argument&) {
			rejected = true;
		}
		check(rejected, "Accepted nonfinite query");
		index->reset(1);
		out.clear();
		index->query(moved, out);
		check(out.empty(), "Reset retained proxies");
		std::cout << N << "D index=" << algorithm << " candidates=" << pairs.size()
				  << " validation_ms=" << std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count() << '\n';
	}
	S::AabbBvh<N, int> tree;
	tree.upsert(0, boxes[1]);
	tree.upsert(1, boxes[2]);
	auto copy = tree;
	tree.clear();
	auto moved = std::move(copy);
	int visits = 0, tests = 0;
	moved.query(
		[&](const auto&) {
			++tests;
			return S::Intersection::Inside;
		},
		[&](auto) { ++visits; });
	check(tests == 1 && visits == 2, "BVH inside classification did not prune predicates");
	check(moved.remove(0) && moved.remove(1) && !moved.remove(1), "BVH remove/final leaf lifecycle");
	visits = 0;
	moved.query([](const auto&) { return S::Intersection::Intersect; }, [&](auto) { ++visits; });
	check(!visits, "Empty BVH returned payload");
}
int main() {
	try {
		spatial<2>();
		spatial<3>();
		std::cout << "Spatial tests passed with physics disabled\n";
		return 0;
	} catch (const std::exception& e) {
		std::cerr << e.what() << '\n';
		return 1;
	}
}

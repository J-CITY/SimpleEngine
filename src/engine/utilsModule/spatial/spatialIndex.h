#pragma once
#include "bvh.h"
#include "spatialGrid.h"
#include <memory>

namespace IKIGAI::UTILS::SPATIAL {
template <std::size_t N, class Id = std::uint64_t> class ISpatialIndex {
public:
	using Bounds = MATH::GEOMETRY::Aabb<N>;
	virtual ~ISpatialIndex() = default;
	// Reset replaces all proxies. Cell size is a tuning hint for grid backends.
	virtual void reset(float cellSize) = 0;
	virtual void upsert(Id id, const Bounds& bounds) = 0;
	virtual bool remove(Id id) = 0;
	virtual void query(const Bounds& bounds, std::vector<Id>& out) const = 0;
	virtual void pairs(std::vector<std::pair<Id, Id>>& out) const = 0;
};
template <std::size_t N, class Id = std::uint64_t> class GridIndex final : public ISpatialIndex<N, Id> {
	SpatialGrid<N, Id> mGrid;

public:
	using Bounds = typename ISpatialIndex<N, Id>::Bounds;
	explicit GridIndex(float size = 2)
		: mGrid(size) { }
	void reset(float size) override {
		mGrid.reset(size);
	}
	void upsert(Id id, const Bounds& b) override {
		mGrid.upsert(id, b);
	}
	bool remove(Id id) override {
		return mGrid.remove(id);
	}
	void query(const Bounds& b, std::vector<Id>& out) const override {
		mGrid.query(b, out);
	}
	void pairs(std::vector<std::pair<Id, Id>>& out) const override {
		mGrid.pairs(out);
	}
};
template <std::size_t N, class Id = std::uint64_t> class BvhIndex final : public ISpatialIndex<N, Id> {
	AabbBvh<N, Id> mTree;
	std::map<Id, typename ISpatialIndex<N, Id>::Bounds> mEntries;

public:
	using Bounds = typename ISpatialIndex<N, Id>::Bounds;
	void reset(float) override {
		mTree.clear();
		mEntries.clear();
	}
	void upsert(Id id, const Bounds& b) override {
		mTree.upsert(id, b);
		mEntries.insert_or_assign(id, b);
	}
	bool remove(Id id) override {
		mEntries.erase(id);
		return mTree.remove(id);
	}
	void query(const Bounds& b, std::vector<Id>& out) const override {
		b.validate();
		mTree.query(
			[&](const auto& v) { return v.overlaps(b) ? Intersection::Intersect : Intersection::Outside; }, [&](Id id) { out.push_back(id); });
	}
	void pairs(std::vector<std::pair<Id, Id>>& out) const override {
		std::vector<Id> candidates;
		for (const auto& [id, b] : mEntries) {
			candidates.clear();
			query(b, candidates);
			for (auto other : candidates)
				if (id < other)
					out.emplace_back(id, other);
		}
	}
};
template <std::size_t N, class Id = std::uint64_t> class BruteForceIndex final : public ISpatialIndex<N, Id> {
	std::map<Id, typename ISpatialIndex<N, Id>::Bounds> mEntries;

public:
	using Bounds = typename ISpatialIndex<N, Id>::Bounds;
	void reset(float) override {
		mEntries.clear();
	}
	void upsert(Id id, const Bounds& b) override {
		b.validate();
		mEntries.insert_or_assign(id, b);
	}
	bool remove(Id id) override {
		return mEntries.erase(id) != 0;
	}
	void query(const Bounds& b, std::vector<Id>& out) const override {
		b.validate();
		for (const auto& [id, v] : mEntries)
			if (b.overlaps(v))
				out.push_back(id);
	}
	void pairs(std::vector<std::pair<Id, Id>>& out) const override {
		for (auto a = mEntries.begin(); a != mEntries.end(); ++a)
			for (auto b = std::next(a); b != mEntries.end(); ++b)
				if (a->second.overlaps(b->second))
					out.emplace_back(a->first, b->first);
	}
};
template <std::size_t N, class Id = std::uint64_t> struct INeighborSearchFactory {
	virtual ~INeighborSearchFactory() = default;
	virtual std::unique_ptr<ISpatialIndex<N, Id>> create(float cellSize) const = 0;
};
template <std::size_t N, class Index = GridIndex<N>, class Id = std::uint64_t> struct NeighborSearchFactory final : INeighborSearchFactory<N, Id> {
	std::unique_ptr<ISpatialIndex<N, Id>> create(float cellSize) const override {
		auto result = std::make_unique<Index>();
		result->reset(cellSize);
		return result;
	}
};
// Kernel-neutral adapter for the existing vector APIs. Used by particle jobs;
// the factory creates a separate index for each data set and iteration.
template <std::size_t N, class Vec, class Id = std::uint64_t> class SpatialQuery {
	std::unique_ptr<ISpatialIndex<N, Id>> mIndex;
	static float coordinate(const Vec& v, std::size_t i) {
		if constexpr (requires { v[i]; })
			return v[i];
		else if constexpr (N == 2)
			return i ? v.y : v.x;
		else {
			static_assert(N == 3, "Provide an indexable vector for this dimension");
			return i == 0 ? v.x : i == 1 ? v.y : v.z;
		}
	}

public:
	using Bounds = MATH::GEOMETRY::Aabb<N>;
	SpatialQuery(const INeighborSearchFactory<N, Id>& factory, float size)
		: mIndex(factory.create(size)) {
		if (!mIndex)
			throw std::invalid_argument("Neighbor factory returned null");
	}
	static Bounds bounds(Vec low, Vec high) {
		Bounds b;
		for (std::size_t i = 0; i < N; ++i) {
			b.min[i] = coordinate(low, i);
			b.max[i] = coordinate(high, i);
		}
		return b;
	}
	ISpatialIndex<N, Id>& index() {
		return *mIndex;
	}
	void insert(Id id, Vec low, Vec high) {
		mIndex->upsert(id, bounds(low, high));
	}
	template <class Box> void insert(Id id, const Box& b) {
		insert(id, b.min, b.max);
	}
	template <class Box> std::vector<Id> query(const Box& b) const {
		std::vector<Id> out;
		mIndex->query(bounds(b.min, b.max), out);
		canonical(out);
		return out;
	}
	std::vector<Id> query(Vec p, float radius) const {
		if (!std::isfinite(radius) || radius < 0)
			throw std::invalid_argument("Invalid query radius");
		Bounds b;
		for (std::size_t i = 0; i < N; ++i) {
			b.min[i] = coordinate(p, i) - radius;
			b.max[i] = coordinate(p, i) + radius;
		}
		std::vector<Id> out;
		mIndex->query(b, out);
		canonical(out);
		return out;
	}
	std::vector<std::pair<Id, Id>> pairs() const {
		std::vector<std::pair<Id, Id>> out;
		mIndex->pairs(out);
		for (auto& p : out)
			if (p.first > p.second)
				std::swap(p.first, p.second);
		out.erase(std::remove_if(out.begin(), out.end(), [](auto p) { return p.first == p.second; }), out.end());
		canonical(out);
		return out;
	}

private:
	template <class T> static void canonical(std::vector<T>& out) {
		std::sort(out.begin(), out.end());
		out.erase(std::unique(out.begin(), out.end()), out.end());
	}
};
}

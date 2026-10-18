#pragma once
#include "mathModule/geometry/aabb.h"
#include <numeric>
#include <vector>

namespace IKIGAI::UTILS::SPATIAL {
enum class Intersection { Outside, Intersect, Inside };
// Value-owned, balanced BVH. Mutations invalidate the tree; the next query
// rebuilds it once. No physics, renderer, pointer payload or ownership assumptions.
template <class Volume, class Payload, class Traits> class Bvh {
	struct Entry {
		Payload payload;
		Volume volume;
	};
	struct Node {
		Volume volume;
		std::size_t left = 0, right = 0, entry = 0;
		bool leaf = false;
	};
	std::vector<Entry> mEntries;
	mutable std::vector<Node> mNodes;
	mutable bool mDirty = true;
	std::size_t build(std::vector<std::size_t>& order, std::size_t first, std::size_t last, std::size_t depth) const {
		auto index = mNodes.size();
		mNodes.push_back({ });
		if (last - first == 1) {
			mNodes[index] = { mEntries[order[first]].volume, 0, 0, order[first], true };
			return index;
		}
		auto middle = first + (last - first) / 2;
		std::nth_element(order.begin() + first, order.begin() + middle, order.begin() + last, [&](auto a, auto b) {
			auto ca = Traits::center(mEntries[a].volume, depth % Traits::dimensions);
			auto cb = Traits::center(mEntries[b].volume, depth % Traits::dimensions);
			return ca == cb ? a < b : ca < cb;
		});
		auto left = build(order, first, middle, depth + 1), right = build(order, middle, last, depth + 1);
		mNodes[index] = { Traits::merge(mNodes[left].volume, mNodes[right].volume), left, right, 0, false };
		return index;
	}
	void prepare() const {
		if (!mDirty)
			return;
		mNodes.clear();
		mNodes.reserve(mEntries.size() * 2);
		std::vector<std::size_t> order(mEntries.size());
		std::iota(order.begin(), order.end(), 0);
		if (!order.empty())
			build(order, 0, order.size(), 0);
		mDirty = false;
	}

public:
	void clear() {
		mEntries.clear();
		mNodes.clear();
		mDirty = true;
	}
	std::size_t size() const {
		return mEntries.size();
	}
	void upsert(const Payload& payload, const Volume& volume) {
		Traits::validate(volume);
		for (auto& e : mEntries)
			if (e.payload == payload) {
				e.volume = volume;
				mDirty = true;
				return;
			}
		mEntries.push_back({ payload, volume });
		mDirty = true;
	}
	bool remove(const Payload& payload) {
		auto i = std::find_if(mEntries.begin(), mEntries.end(), [&](const auto& e) { return e.payload == payload; });
		if (i == mEntries.end())
			return false;
		mEntries.erase(i);
		mDirty = true;
		return true;
	}
	template <class Predicate, class Visitor> void query(Predicate predicate, Visitor visitor) const {
		if (mEntries.empty())
			return;
		prepare();
		struct Visit {
			std::size_t node;
			bool inside;
		};
		std::vector<Visit> stack { { 0, false } };
		while (!stack.empty()) {
			auto [index, inside] = stack.back();
			stack.pop_back();
			const auto& n = mNodes[index];
			auto classification = inside ? Intersection::Inside : predicate(n.volume);
			if (classification == Intersection::Outside)
				continue;
			if (n.leaf)
				visitor(mEntries[n.entry].payload);
			else {
				stack.push_back({ n.right, classification == Intersection::Inside });
				stack.push_back({ n.left, classification == Intersection::Inside });
			}
		}
	}
};
template <std::size_t N> struct BoundsTraits : MATH::GEOMETRY::AabbTraits<N> {
	static constexpr std::size_t dimensions = N;
};
template <std::size_t N, class Payload> using AabbBvh = Bvh<MATH::GEOMETRY::Aabb<N>, Payload, BoundsTraits<N>>;
}

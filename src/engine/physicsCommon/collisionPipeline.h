#pragma once
#include "utilsModule/spatial/spatialIndex.h"

namespace IKIGAI::PHYSICS_COMMON {
template <std::size_t N, class NarrowPhase, class DefaultNarrowPhase> struct CollisionPipeline {
	using Index = UTILS::SPATIAL::ISpatialIndex<N>;
	using NeighborFactory = UTILS::SPATIAL::INeighborSearchFactory<N>;
	std::unique_ptr<Index> broadPhase = std::make_unique<UTILS::SPATIAL::GridIndex<N>>();
	std::unique_ptr<NarrowPhase> narrowPhase = std::make_unique<DefaultNarrowPhase>();
	std::unique_ptr<NeighborFactory> neighbors = std::make_unique<UTILS::SPATIAL::NeighborSearchFactory<N>>();
	void validate() const {
		if (!broadPhase || !narrowPhase || !neighbors)
			throw std::invalid_argument("Collision pipeline contains null strategy");
	}
};
template <class Id> void canonicalPairs(std::vector<std::pair<Id, Id>>& pairs) {
	for (auto& p : pairs)
		if (p.first > p.second)
			std::swap(p.first, p.second);
	std::erase_if(pairs, [](auto p) { return p.first == p.second; });
	std::sort(pairs.begin(), pairs.end());
	pairs.erase(std::unique(pairs.begin(), pairs.end()), pairs.end());
}
}

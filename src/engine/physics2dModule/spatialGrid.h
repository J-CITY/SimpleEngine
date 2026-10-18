#pragma once
#include "physics2d.h"
#include "utilsModule/spatial/spatialIndex.h"
namespace IKIGAI::PHYSICS2D {
class SpatialGrid : public UTILS::SPATIAL::SpatialQuery<2, Vec2, Id> {
	using Base = UTILS::SPATIAL::SpatialQuery<2, Vec2, Id>;
	static const auto& defaultFactory() {
		static GridNeighborSearchFactory2D f;
		return f;
	}

public:
	explicit SpatialGrid(float size)
		: Base(defaultFactory(), size) { }
	SpatialGrid(const INeighborSearch2D& factory, float size)
		: Base(factory, size) { }
	using Base::insert;
	void insert(Id id, Aabb box) {
		Base::insert(id, box);
	}
	std::vector<Id> query(Aabb box) const {
		return Base::query(box);
	}
};
}

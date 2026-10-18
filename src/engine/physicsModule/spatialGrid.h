#pragma once
#include "physicWorld.h"
#include "utilsModule/spatial/spatialIndex.h"
namespace IKIGAI::PHYSICS {
class SpatialGrid : public UTILS::SPATIAL::SpatialQuery<3, Vec3, Id> {
	using Base = UTILS::SPATIAL::SpatialQuery<3, Vec3, Id>;
	// Public compatibility wrapper; particle jobs use the world's factory.
	static const auto& defaultFactory() {
		static UTILS::SPATIAL::NeighborSearchFactory<3, UTILS::SPATIAL::GridIndex<3, Id>, Id> f;
		return f;
	}

public:
	explicit SpatialGrid(float size)
		: Base(defaultFactory(), size) { }
	SpatialGrid(const INeighborSearch3D& factory, float size)
		: Base(factory, size) { }
};
}

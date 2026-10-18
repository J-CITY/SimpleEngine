#pragma once
#include "collisionModule/collision2d.h"
#include "physicsCommon/collisionPipeline.h"
namespace IKIGAI::PHYSICS2D {
using IBroadPhase2D = UTILS::SPATIAL::ISpatialIndex<2>;
using GridBroadPhase2D = UTILS::SPATIAL::GridIndex<2>;
using BvhBroadPhase2D = UTILS::SPATIAL::BvhIndex<2>;
using BruteForceBroadPhase2D = UTILS::SPATIAL::BruteForceIndex<2>;
using INarrowPhase2D = COLLISION::INarrowPhase2D;
using DefaultNarrowPhase2D = COLLISION::DefaultNarrowPhase2D;
using INeighborSearch2D = UTILS::SPATIAL::INeighborSearchFactory<2>;
using GridNeighborSearchFactory2D = UTILS::SPATIAL::NeighborSearchFactory<2>;
using BvhNeighborSearchFactory2D = UTILS::SPATIAL::NeighborSearchFactory<2, BvhBroadPhase2D>;
using CollisionPipeline = PHYSICS_COMMON::CollisionPipeline<2, INarrowPhase2D, DefaultNarrowPhase2D>;
}

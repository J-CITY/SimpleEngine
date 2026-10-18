#pragma once
#include "collisionModule/collision3d.h"
#include "physicsCommon/collisionPipeline.h"
namespace IKIGAI::PHYSICS {
using IBroadPhase3D = UTILS::SPATIAL::ISpatialIndex<3>;
using GridBroadPhase3D = UTILS::SPATIAL::GridIndex<3>;
using BvhBroadPhase3D = UTILS::SPATIAL::BvhIndex<3>;
using BruteForceBroadPhase3D = UTILS::SPATIAL::BruteForceIndex<3>;
using INarrowPhase3D = COLLISION::INarrowPhase3D;
using DefaultNarrowPhase3D = COLLISION::DefaultNarrowPhase3D;
using INeighborSearch3D = UTILS::SPATIAL::INeighborSearchFactory<3>;
using GridNeighborSearchFactory3D = UTILS::SPATIAL::NeighborSearchFactory<3>;
using BvhNeighborSearchFactory3D = UTILS::SPATIAL::NeighborSearchFactory<3, BvhBroadPhase3D>;
using CollisionPipeline = PHYSICS_COMMON::CollisionPipeline<3, INarrowPhase3D, DefaultNarrowPhase3D>;
}

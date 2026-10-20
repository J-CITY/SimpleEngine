#pragma once
#if USE_PHYSICS_3D
#include "mathModule/math.h"
namespace IKIGAI::PHYSICS {
class PhysicWorld;
}
namespace IKIGAI::SCENE_SYSTEM {
class Scene;
}
namespace IKIGAI::ECS {
class CharacterMotorComponent;
class TraversalComponent;
class TraversalSystem {
public:
	static void prepare(TraversalComponent&);
	static bool tryStart(const PHYSICS::PhysicWorld&, CharacterMotorComponent&, TraversalComponent&, MATH::Vector3f);
	static void update(SCENE_SYSTEM::Scene&, CharacterMotorComponent&, TraversalComponent&, float);
};
}
#endif

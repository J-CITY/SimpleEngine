#pragma once
#if USE_PHYSICS_3D
namespace IKIGAI::SCENE_SYSTEM {
class Scene;
}
namespace IKIGAI::ECS {
class CharacterSystem {
public:
	static void registerProperties();
	static void fixedUpdate(SCENE_SYSTEM::Scene&, float);
	static void update(SCENE_SYSTEM::Scene&, float);
};
}
#endif

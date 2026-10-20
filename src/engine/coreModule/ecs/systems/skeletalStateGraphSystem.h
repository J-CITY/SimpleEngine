#pragma once
namespace IKIGAI::SCENE_SYSTEM {
class Scene;
}
namespace IKIGAI::ECS {
class SkeletalStateGraphSystem {
public:
	static void update(SCENE_SYSTEM::Scene&, float);
};
}

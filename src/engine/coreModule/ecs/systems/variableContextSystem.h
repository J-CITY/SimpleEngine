#pragma once
namespace IKIGAI::SCENE_SYSTEM {
class Scene;
}
namespace IKIGAI::ECS {
class VariableContextSystem {
public:
	static void update(SCENE_SYSTEM::Scene&);
};
}

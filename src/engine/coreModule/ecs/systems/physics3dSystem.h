#pragma once
#if USE_PHYSICS_3D
#include "ecsModule/systemManager.h"
#include "physicsModule/physicWorld.h"
#include <memory>
namespace IKIGAI::SCENE_SYSTEM {
class Scene;
}
namespace IKIGAI::ECS {
class Physics3DSystem final : public ECS2::System {
public:
	explicit Physics3DSystem(SCENE_SYSTEM::Scene& scene, std::shared_ptr<PHYSICS::World> simulation = std::make_shared<PHYSICS::World>());
	void onFixedUpdate(ECS2::World&, ECS2::CommandBuffer&, std::chrono::duration<double>) override;
	PHYSICS::World& simulation() {
		return *mSimulation;
	}
	bool paused = false, singleStep = false;
#ifdef USE_EDITOR
	void drawDebug();
#endif
private:
	SCENE_SYSTEM::Scene& mScene;
	std::shared_ptr<PHYSICS::World> mSimulation;
};
void RunPhysics3DSmoke();
}

#endif // USE_PHYSICS_3D

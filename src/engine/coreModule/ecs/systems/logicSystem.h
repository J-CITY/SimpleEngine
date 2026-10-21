#pragma once

#include "ecsModule/systemManager.h"

namespace IKIGAI::ECS {
class Object;
}

namespace IKIGAI::ECS {
class LogicSystem : public ECS::System {
public:
	LogicSystem();
	void onAwake(ECS::World& world) override;
	void onStart(ECS::World& world) override;
	void onEnable(ECS::World& world) override;
	void onDisable(ECS::World& world) override;
	void onDestroy(ECS::World& world) override;
	void onUpdate(ECS::World& world, ECS::CommandBuffer& cb, std::chrono::duration<double> dt) override;
	void onFixedUpdate(ECS::World& world, ECS::CommandBuffer& cb, std::chrono::duration<double> dt) override;
	void onLateUpdate(ECS::World& world, ECS::CommandBuffer& cb, std::chrono::duration<double> dt) override;
};
}

#pragma once
#include "../systemManager.h"
#include "ecsModule/systemManager.h"

namespace IKIGAI::ECS { class Object; }

namespace IKIGAI::ECS {
	class LogicSystem : public ECS2::System {
	public:
		LogicSystem();
		void onAwake(ECS2::World& world) override;
		void onStart(ECS2::World& world) override;
		void onEnable(ECS2::World& world) override;
		void onDisable(ECS2::World& world) override;
		void onDestroy(ECS2::World& world) override;
		void onUpdate(ECS2::World& world, ECS2::CommandBuffer& cb, std::chrono::duration<double> dt) override;
		void onFixedUpdate(ECS2::World& world, ECS2::CommandBuffer& cb, std::chrono::duration<double> dt) override;
		void onLateUpdate(ECS2::World& world, ECS2::CommandBuffer& cb, std::chrono::duration<double> dt) override;
	};
}
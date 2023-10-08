#pragma once
#include "../systemManager.h"

namespace IKIGAI::ECS { class Object; }

namespace IKIGAI::ECS {
	class LogicSystem : public System {
	public:
		LogicSystem();
		void onAwake() override;
		void onStart() override;
		void onEnable() override;
		void onDisable() override;
		void onDestroy() override;
		void onUpdate(std::chrono::duration<double> dt) override;
		void onFixedUpdate(std::chrono::duration<double> dt) override;
		void onLateUpdate(std::chrono::duration<double> dt) override;
	};
}
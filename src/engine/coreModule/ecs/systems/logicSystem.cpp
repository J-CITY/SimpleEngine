#include "logicSystem.h"
#include <sceneModule/sceneManager.h>

IKIGAI::ECS::LogicSystem::LogicSystem() {
}

void IKIGAI::ECS::LogicSystem::onAwake() {
	for (auto& component : ECS::ComponentManager::GetInstance().getComponentArrayRef<ECS::LogicComponent>()) {
		component._onAwake();
	}
}

void IKIGAI::ECS::LogicSystem::onStart() {
	for (auto& component : ECS::ComponentManager::GetInstance().getComponentArrayRef<ECS::LogicComponent>()) {
		component._onStart();
	}
}

void IKIGAI::ECS::LogicSystem::onEnable() {
	for (auto& component : ECS::ComponentManager::GetInstance().getComponentArrayRef<ECS::LogicComponent>()) {
		component._onEnable();
	}
}

void IKIGAI::ECS::LogicSystem::onDisable() {
	for (auto& component : ECS::ComponentManager::GetInstance().getComponentArrayRef<ECS::LogicComponent>()) {
		component._onDisable();
	}
}

void IKIGAI::ECS::LogicSystem::onDestroy() {
	for (auto& component : ECS::ComponentManager::GetInstance().getComponentArrayRef<ECS::LogicComponent>()) {
		component._onDestroy();
	}
}

void IKIGAI::ECS::LogicSystem::onUpdate(std::chrono::duration<double> dt) {
	for (auto& component : ECS::ComponentManager::GetInstance().getComponentArrayRef<ECS::LogicComponent>()) {
		component._onUpdate(dt);
	}
}

void IKIGAI::ECS::LogicSystem::onFixedUpdate(std::chrono::duration<double> dt) {
	for (auto& component : ECS::ComponentManager::GetInstance().getComponentArrayRef<ECS::LogicComponent>()) {
		component._onFixedUpdate(dt);
	}
}

void IKIGAI::ECS::LogicSystem::onLateUpdate(std::chrono::duration<double> dt) {
	for (auto& component : ECS::ComponentManager::GetInstance().getComponentArrayRef<ECS::LogicComponent>()) {
		component._onLateUpdate(dt);
	}
}

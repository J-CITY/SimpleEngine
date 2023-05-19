#include "logicSystem.h"
#include "../../scene/sceneManager.h"

KUMA::ECS::LogicSystem::LogicSystem() {
}

void KUMA::ECS::LogicSystem::onAwake() {
	for (auto& component : ECS::ComponentManager::getInstance()->getComponentArrayRef<ECS::LogicComponent>()) {
		component._onAwake();
	}
}

void KUMA::ECS::LogicSystem::onStart() {
	for (auto& component : ECS::ComponentManager::getInstance()->getComponentArrayRef<ECS::LogicComponent>()) {
		component._onStart();
	}
}

void KUMA::ECS::LogicSystem::onEnable() {
	for (auto& component : ECS::ComponentManager::getInstance()->getComponentArrayRef<ECS::LogicComponent>()) {
		component._onEnable();
	}
}

void KUMA::ECS::LogicSystem::onDisable() {
	for (auto& component : ECS::ComponentManager::getInstance()->getComponentArrayRef<ECS::LogicComponent>()) {
		component._onDisable();
	}
}

void KUMA::ECS::LogicSystem::onDestroy() {
	for (auto& component : ECS::ComponentManager::getInstance()->getComponentArrayRef<ECS::LogicComponent>()) {
		component._onDestroy();
	}
}

void KUMA::ECS::LogicSystem::onUpdate(std::chrono::duration<double> dt) {
	for (auto& component : ECS::ComponentManager::getInstance()->getComponentArrayRef<ECS::LogicComponent>()) {
		component._onUpdate(dt);
	}
}

void KUMA::ECS::LogicSystem::onFixedUpdate(std::chrono::duration<double> dt) {
	for (auto& component : ECS::ComponentManager::getInstance()->getComponentArrayRef<ECS::LogicComponent>()) {
		component._onFixedUpdate(dt);
	}
}

void KUMA::ECS::LogicSystem::onLateUpdate(std::chrono::duration<double> dt) {
	for (auto& component : ECS::ComponentManager::getInstance()->getComponentArrayRef<ECS::LogicComponent>()) {
		component._onLateUpdate(dt);
	}
}

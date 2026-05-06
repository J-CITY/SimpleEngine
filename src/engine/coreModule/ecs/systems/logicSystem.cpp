#include "logicSystem.h"
#include <sceneModule/sceneManager.h>

IKIGAI::ECS::LogicSystem::LogicSystem() {
	//mComponentsRead.insert(typeid(LogicComponent).name());
	mName = "LogicSystem";
	auto cm = RESOURCES::ServiceManager::Get<ECS2::World>().getComponentManager();
	mReads.insert(cm->getComponentType<LogicComponent>());
}

void IKIGAI::ECS::LogicSystem::onAwake(ECS2::World& world) {
	for (auto& component : world.getComponentManager()->getComponentsArray<ECS::LogicComponent>()) {
		component._onAwake();
	}
	//for (auto& component : ECS::ComponentManager::GetInstance().getComponentArrayRef<ECS::LogicComponent>()) {
	//	component._onAwake();
	//}
}

void IKIGAI::ECS::LogicSystem::onStart(ECS2::World& world) {
	for (auto& component : world.getComponentManager()->getComponentsArray<ECS::LogicComponent>()) {
		component._onStart();
	}
	//for (auto& component : ECS::ComponentManager::GetInstance().getComponentArrayRef<ECS::LogicComponent>()) {
	//	component._onStart();
	//}
}

void IKIGAI::ECS::LogicSystem::onEnable(ECS2::World& world) {
	for (auto& component : world.getComponentManager()->getComponentsArray<ECS::LogicComponent>()) {
		component._onEnable();
	}
	//for (auto& component : ECS::ComponentManager::GetInstance().getComponentArrayRef<ECS::LogicComponent>()) {
	//	component._onEnable();
	//}
}

void IKIGAI::ECS::LogicSystem::onDisable(ECS2::World& world) {
	for (auto& component : world.getComponentManager()->getComponentsArray<ECS::LogicComponent>()) {
		component._onDisable();
	}
	//for (auto& component : ECS::ComponentManager::GetInstance().getComponentArrayRef<ECS::LogicComponent>()) {
	//	component._onDisable();
	//}
}

void IKIGAI::ECS::LogicSystem::onDestroy(ECS2::World& world) {
	for (auto& component : world.getComponentManager()->getComponentsArray<ECS::LogicComponent>()) {
		component._onDestroy();
	}
	//for (auto& component : ECS::ComponentManager::GetInstance().getComponentArrayRef<ECS::LogicComponent>()) {
	//	component._onDestroy();
	//}
}

void IKIGAI::ECS::LogicSystem::onUpdate(ECS2::World& world, ECS2::CommandBuffer& cb, std::chrono::duration<double> dt) {
	for (auto& component : world.getComponentManager()->getComponentsArray<ECS::LogicComponent>()) {
		component._onUpdate(dt);
	}
	//for (auto& component : ECS::ComponentManager::GetInstance().getComponentArrayRef<ECS::LogicComponent>()) {
	//	component._onUpdate(dt);
	//}
}

void IKIGAI::ECS::LogicSystem::onFixedUpdate(ECS2::World& world, ECS2::CommandBuffer& cb, std::chrono::duration<double> dt) {
	for (auto& component : world.getComponentManager()->getComponentsArray<ECS::LogicComponent>()) {
		component._onFixedUpdate(dt);
	}
	//for (auto& component : ECS::ComponentManager::GetInstance().getComponentArrayRef<ECS::LogicComponent>()) {
	//	component._onFixedUpdate(dt);
	//}
}

void IKIGAI::ECS::LogicSystem::onLateUpdate(ECS2::World& world, ECS2::CommandBuffer& cb, std::chrono::duration<double> dt) {
	for (auto& component : world.getComponentManager()->getComponentsArray<ECS::LogicComponent>()) {
		component._onLateUpdate(dt);
	}
	//for (auto& component : ECS::ComponentManager::GetInstance().getComponentArrayRef<ECS::LogicComponent>()) {
	//	component._onLateUpdate(dt);
	//}
}

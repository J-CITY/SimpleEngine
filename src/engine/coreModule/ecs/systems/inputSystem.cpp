#include "inputSystem.h"
#include <sceneModule/sceneManager.h>

IKIGAI::ECS::InputSystem::InputSystem() {
	//mComponentsRead.insert(typeid(InputComponent).name());
	mName = "InputSystem";
	auto cm = RESOURCES::ServiceManager::Get<ECS2::World>().getComponentManager();
	mReads.insert(cm->getComponentType<InputComponent>());
}

void IKIGAI::ECS::InputSystem::onUpdate(ECS2::World& world, ECS2::CommandBuffer& cb, std::chrono::duration<double> dt) {
	//world.getComponentManager()->forEach<ECS::InputComponent>(
	//	[&](IKIGAI::ECS2::Entity e, ECS::InputComponent& component) {
	//		if (component.getActive()) {
	//			component.getEventFunc()(dt);
	//		}
	//	}
	//);
	for (auto& component : world.getComponentManager()->getComponentsArray<ECS::InputComponent>()) {
		if (component.getActive()) {
			component.getEventFunc()(dt);
		}
	}
	//for (auto& component : ECS::ComponentManager::GetInstance().getComponentArrayRef<ECS::InputComponent>()) {
	//	if (component.getActive()) {
	//		component.getEventFunc()(dt);
	//	}
	//}
}


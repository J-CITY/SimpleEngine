#include "inputSystem.h"
#include <sceneModule/sceneManager.h>

IKIGAI::ECS::InputSystem::InputSystem() {
	mComponentsRead.insert(typeid(InputComponent).name());
}

void IKIGAI::ECS::InputSystem::onUpdate(std::chrono::duration<double> dt) {
	for (auto& component : ECS::ComponentManager::GetInstance().getComponentArrayRef<ECS::InputComponent>()) {
		if (component.getActive()) {
			component.getEventFunc()(dt);
		}
	}
}


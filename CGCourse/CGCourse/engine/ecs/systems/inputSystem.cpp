#include "inputSystem.h"
#include "../../scene/sceneManager.h"

KUMA::ECS::InputSystem::InputSystem() {

}

void KUMA::ECS::InputSystem::onUpdate(std::chrono::duration<double> dt) {
	for (auto& component : ECS::ComponentManager::getInstance()->getComponentArrayRef<ECS::InputComponent>()) {
		if (component.getActive()) {
			component.getEventFunc()(dt);
		}
	}
}


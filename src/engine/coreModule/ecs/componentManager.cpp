#include "componentManager.h"

using namespace IKIGAI::ECS;


ComponentManager::ComponentManager() {
	systemManager = std::make_unique<SystemManager>();
}

void ComponentManager::setSignature(Entity entity, Signature signature) {
	signatures[entity] = signature;
}

Signature ComponentManager::getSignature(Entity entity) {
#ifdef __DEBUG__
			if (!signatures.contains(entity)) {
				throw;
			}
#endif
	return signatures[entity];
}

SystemManager& ComponentManager::getSystemManager() {
	return *systemManager;
}

void ComponentManager::entityDestroyed(Entity entity) {
	for (auto const& pair : componentArrays) {
		auto const& components = pair.second;
		components->entityDestroyed(entity);
	}
	systemManager->entityDestroyed(entity);
}

void ComponentManager::enable(Entity id) {
	enabledComponent(id, ComponentsTypeProviderType{});
}

void ComponentManager::disable(Entity id) {
	disableComponent(id, ComponentsTypeProviderType{});
}

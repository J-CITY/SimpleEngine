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
/*
void IKIGAI::ECS::TestECS() {
	ECS::ComponentManager::GetInstance().registerComponentArchetype<PosC>();
	ECS::ComponentManager::GetInstance().registerComponentArchetype<ColorC>();
	ECS::ComponentManager::GetInstance().registerComponentArchetype<NameC>();

	ECS::ComponentManager::GetInstance().getSystemManager().registerSystem<SystemA>();
	ECS::ComponentManager::GetInstance().getSystemManager().registerSystem<SystemB>();

	auto e0 = Entity(0);
	auto e1 = Entity(1);
	auto e2 = Entity(2);

	ECS::ComponentManager::GetInstance().createEntity(e0);
	ECS::ComponentManager::GetInstance().createEntity(e1);
	ECS::ComponentManager::GetInstance().createEntity(e2);

	SystemA sa;
	SystemB sb;

	{
		PosC c0 = PosC(1, 1);
		ColorC c1 = ColorC(255, 111, 255);
		NameC c2 = NameC("e0");
		ECS::ComponentManager::GetInstance().addComponentArchetype(e0, c0);
		ECS::ComponentManager::GetInstance().addComponentArchetype(e0, c1);
		ECS::ComponentManager::GetInstance().addComponentArchetype(e0, c2);
	}
	{
		PosC c0 = PosC(2, 2);
		ColorC c1 = ColorC(100, 255, 255);
		NameC c2 = NameC("e1");
		ECS::ComponentManager::GetInstance().addComponentArchetype(e1, c0);
		//ECS::ComponentManager::GetInstance().addComponentArchetype(e0, c1);
		ECS::ComponentManager::GetInstance().addComponentArchetype(e1, c2);
	}
	{
		PosC c0 = PosC(12, 22);
		ColorC c1 = ColorC(100, 255, 111);
		NameC c2 = NameC("e2");
		ECS::ComponentManager::GetInstance().addComponentArchetype(e2, c0);
		//ECS::ComponentManager::GetInstance().addComponentArchetype(e0, c1);
		ECS::ComponentManager::GetInstance().addComponentArchetype(e2, c2);
	}

	sa.update();
	std::cout << "===================\n";
	sb.update();
	std::cout << "===================\n";

	ECS::ComponentManager::GetInstance().removeComponentArchetype<PosC>(e2);
	ECS::ComponentManager::GetInstance().removeComponentArchetype<NameC>(e2);

	sa.update();
	std::cout << "===================\n";
	sb.update();
	std::cout << "===================\n";

}
*/

#include "world.h"

#include "componentManager.h"

namespace IKIGAI::ECS2
{
	World::World() {
		mComponentsManager = std::make_unique<ComponentManager>();
		mSystemsManager = std::make_unique<SystemManager>(*this);
	}

	World::~World() = default;

	ComponentManager* World::getComponentManager() const noexcept {
		return mComponentsManager.get();
	}

	Entity World::createEntity() const {
		Entity newEntity;
		mComponentsManager->registerEntity(newEntity);
		return newEntity;
	}

	void World::registerEntity(Entity newEntity) const {
		mComponentsManager->registerEntity(newEntity);
	}

	void World::destroyEntity(Entity newEntity) const {
		mComponentsManager->destroyEntity(newEntity);
	}

	SystemManager* World::getSystemManager() const noexcept {
		return mSystemsManager.get();
	}
}

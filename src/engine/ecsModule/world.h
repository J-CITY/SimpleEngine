#pragma once
#include <memory>

#include "entityManager.h"
#include "utilsModule/idGenerator.h"
#include <string>

namespace IKIGAI::ECS2
{
	class SystemManager;
	class ComponentManager;
	template<typename... Components>
	class SystemBuilder;

	class World : public IdGenerator<World> {
	public:
		World();
		~World();

		[[nodiscard]] ComponentManager* getComponentManager() const noexcept;
		[[nodiscard]] SystemManager* getSystemManager() const noexcept;

		template<typename... T>
		SystemBuilder<T...> system(std::string name);

		Entity createEntity() const;
		void registerEntity(Entity newEntity) const;
		void destroyEntity(Entity newEntity) const;

	private:
		std::unique_ptr<SystemManager> mSystemsManager;
		std::unique_ptr<ComponentManager> mComponentsManager;
	};
}

#include "systemBuilder.h"
#include "systemManager.h"

namespace IKIGAI::ECS2 {
	template<typename... T>
	SystemBuilder<T...> World::system(std::string name) {
		return SystemBuilder<T...>(*getSystemManager(), name);
	}
}

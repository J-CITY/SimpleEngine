#pragma once
#include <memory>
#include "systemManager.h"


#include "entityManager.h"
#include "utilsModule/idGenerator.h"

namespace IKIGAI::ECS2
{
	class SystemManager;
	class ComponentManager;

	class World : public IdGenerator<World> {
	public:
		World();
		~World();

		[[nodiscard]] ComponentManager* getComponentManager() const noexcept;
		[[nodiscard]] SystemManager* getSystemManager() const noexcept;

		template<typename... T>
		auto system(std::string name = "") {
			return SystemBuilder<T...>(*getSystemManager(), name);
		}

		Entity createEntity() const;

	private:
		std::unique_ptr<SystemManager> mSystemsManager;
		std::unique_ptr<ComponentManager> mComponentsManager;
	};
}

#include "systemBuilder.h"

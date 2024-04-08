#pragma once
#include <any>
#include "utilsModule/idGenerator.h"
#include "utilsModule/uniqueAny.h"

namespace IKIGAI::ECS {
	class Object;
	using Entity = Id<Object>;

	class ComponentArrayInterface {
	public:
		virtual ~ComponentArrayInterface() = default;
		virtual void entityDestroyed(Entity entity) = 0;
		virtual std::shared_ptr<ComponentArrayInterface> createEmptyFromThis() = 0;

		//virtual void insertDataAny(Entity entity, UTILS::unique_any&& component) = 0;
		//virtual UTILS::unique_any&& removeDataAny(Entity entity) = 0;
		//virtual std::any getDataAny(Entity entity) = 0;
		[[nodiscard]] virtual int getSize() const = 0;
	};
}

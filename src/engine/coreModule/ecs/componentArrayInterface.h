#pragma once
#include <any>
#include <utilsModule/idObject.h>

namespace IKIGAI::ECS {
	class Object;
	using Entity = ObjectId<Object>;

	class ComponentArrayInterface {
	public:
		virtual ~ComponentArrayInterface() = default;
		virtual void entityDestroyed(Entity entity) = 0;
		virtual std::shared_ptr<ComponentArrayInterface> createEmptyFromThis() = 0;

		virtual void insertDataAny(Entity entity, std::any component) = 0;
		virtual std::any removeDataAny(Entity entity) = 0;
		virtual std::any getDataAny(Entity entity) = 0;
		virtual int getSize() const = 0;
	};
}

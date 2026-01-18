#pragma once
#include <memory>
#include "entityManager.h"

namespace IKIGAI::ECS2
{
	class ComponentArrayInterface {
	public:
		virtual ~ComponentArrayInterface() = default;
		virtual void entityDestroyed(Entity entity) = 0;
		virtual std::shared_ptr<ComponentArrayInterface> createEmptyFromThis() = 0;
		[[nodiscard]] virtual int getSize() const = 0;
		virtual Entity getEntity(size_t index) = 0;
	};
}

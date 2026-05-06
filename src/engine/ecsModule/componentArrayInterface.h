#pragma once
#include <memory>
#include "entityManager.h"
#include "utilsModule/weakPtr.h"

namespace IKIGAI::ECS2 { class Component; }

namespace IKIGAI::ECS2
{
	class ComponentArrayInterface {
	public:
		virtual ~ComponentArrayInterface() = default;
		virtual void entityDestroyed(Entity entity) = 0;
		virtual std::shared_ptr<ComponentArrayInterface> createEmptyFromThis() = 0;
		[[nodiscard]] virtual int getSize() const = 0;
		virtual Entity getEntity(size_t index) = 0;
		[[nodiscard]] virtual bool contains(Entity entity) const = 0;
		virtual UTILS::WeakPtr<ECS2::Component> getComponentBasePtr(Entity entity) = 0;
	};
}

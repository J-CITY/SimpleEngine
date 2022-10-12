#pragma once
#include "../utils/idObject.h"

namespace KUMA::ECS
{
	class Object;
	using Entity = ObjectId<Object>;

	class IComponentArray {
	public:
		virtual ~IComponentArray() = default;
		virtual void entityDestroyed(Entity entity) = 0;
	};
}

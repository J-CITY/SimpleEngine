#pragma once
#include <utilsModule/idObject.h>

namespace IKIGAI::ECS {
	class Object;
	using Entity = ObjectId<Object>;

	class ComponentArrayInterface {
	public:
		virtual ~ComponentArrayInterface() = default;
		virtual void entityDestroyed(Entity entity) = 0;
	};
}

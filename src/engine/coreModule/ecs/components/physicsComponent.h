#pragma once

#include "component.h"
#include <functional>
#include <renderModule/objects/BoundingSphere.h>
#include <physicsModule/body.h>
#include "physicsModule/narrowPhase.h"

import glmath;

namespace IKIGAI
{
	namespace PHYSICS
	{
		class CollisionPrimitive;
	}
}

namespace IKIGAI::ECS { class Object; }

namespace IKIGAI::ECS {
	enum CollisionType {
		NONE = -1,
		SPHERE,
		AABB,
		OBB,
		COUNT,
	};
	class PhysicsComponent : public Component {
	public:
		PhysicsComponent(UTILS::Ref<ECS::Object> obj);
		int GetDirty() const { return m_dirtyFlag; }
		void SetDirty(bool dirtyFlag) { m_dirtyFlag = dirtyFlag; }
		void ResetDirty() { m_dirtyFlag = false; }
		std::shared_ptr<PHYSICS::RigidBody> body;
		IKIGAI::RENDER::BoundingSphere boundingSphere;

		std::shared_ptr<PHYSICS::CollisionPrimitive> collider;
		//std::unique_ptr<PHYSICS::CollisionPrimitive> collider;

		CollisionType collisionType = CollisionType::NONE;
	private:
		bool m_dirtyFlag = false;
	};
}

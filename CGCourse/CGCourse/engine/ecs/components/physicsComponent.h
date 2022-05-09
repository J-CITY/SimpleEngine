#pragma once

#include "component.h"
#include "../../utils/math/Matrix4.h"
#include "../../utils/math/Vector3.h"
#include "../../utils/math/Quaternion.h"
#include <functional>
#include "../../render/objects/BoundingSphere.h"
#include "../../physics/body.h"
#include "../../physics/narrowPhase.h"



namespace KUMA::ECS { class Object; }

namespace KUMA::ECS {
	enum CollisionType {
		NONE = -1,
		SPHERE,
		AABB,
		OBB,
		COUNT,
	};
	class PhysicsComponent : public Component {
	public:
		PhysicsComponent(const Object& obj): Component(obj) {
			
		}
		int GetDirty() const { return m_dirtyFlag; }
		void SetDirty(bool dirtyFlag) { m_dirtyFlag = dirtyFlag; }
		void ResetDirty() { m_dirtyFlag = false; }
		std::shared_ptr<PHYSICS::RigidBody> body;
		KUMA::RENDER::BoundingSphere boundingSphere;

		std::unique_ptr<PHYSICS::CollisionPrimitive> collider;

		CollisionType collisionType = CollisionType::NONE;
	private:
		bool m_dirtyFlag = false;
	};
}

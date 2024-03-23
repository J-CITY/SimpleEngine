#pragma once

#include "component.h"
#include <functional>
#include <renderModule/objects/BoundingSphere.h>
#include <physicsModule/body.h>
#include "physicsModule/narrowPhase.h"

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
		//TODO: add params
		struct Descriptor : public Component::Descriptor {
			std::string Type;

			template<class Context>
			constexpr static auto serde(Context& context, Descriptor& value) {
				using Self = Descriptor;
				using namespace serde::attribute;
				serde::serde_struct(context, value)
					.field(&Self::Type, "PhysicsComponentType");
			}
		};

		PhysicsComponent(UTILS::Ref<ECS::Object> obj);
		PhysicsComponent(UTILS::Ref<ECS::Object> obj, const Component::Descriptor& descriptor) :
			PhysicsComponent(obj) {
		};
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

	public:
		static auto GetMembers() {
			return std::tuple{
			};
		}
	};

	template <>
	inline std::string ECS::GetType<PhysicsComponent>() {
		return "class IKIGAI::ECS::PhysicsComponent";
	}

	template <>
	inline std::string IKIGAI::ECS::GetComponentName<PhysicsComponent>() {
		return "PhysicsComponent";
	}
}

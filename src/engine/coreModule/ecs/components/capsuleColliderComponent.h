#pragma once
#if USE_PHYSICS_3D
#include "coreModule/ecs/componentReference.h"
#include "coreModule/ecs/components/component.h"
#include "physicsModule/characterMotor.h"
namespace IKIGAI::ECS {
class CapsuleColliderComponent : public ComponentBase {
public:
	struct Descriptor : ComponentBase::Descriptor {
		std::string Type;
		float Radius = .3f;
		float Height = 1.8f;
		template <class C> static auto serde(C& c, Descriptor& v) {
			using namespace serde::attribute;
			serde::serde_struct(c, v)
				.field(&Descriptor::Type, "CapsuleColliderComponentType")
				.field(&Descriptor::Radius, "Radius", default_ { float(.3f) })
				.field(&Descriptor::Height, "Height", default_ { float(1.8f) });
		}
	};
	explicit CapsuleColliderComponent(UTILS::Ref<Object>);
	CapsuleColliderComponent(UTILS::Ref<Object>, const ComponentBase::Descriptor& d);
	Descriptor getDescriptor() const {
		return settings;
	}
	static auto GetMembers() {
		return std::tuple {};
	}
	Descriptor settings;
	PHYSICS::CHARACTER::Capsule shape(MATH::Vector3f feet) const;
};
template <> inline std::string GetType<CapsuleColliderComponent>() {
	return "class IKIGAI::ECS::CapsuleColliderComponent";
}
template <> inline std::string GetComponentName<CapsuleColliderComponent>() {
	return "CapsuleColliderComponent";
}
}
#endif

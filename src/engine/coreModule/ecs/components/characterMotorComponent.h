#pragma once
#if USE_PHYSICS_3D
#include "coreModule/ecs/componentReference.h"
#include "coreModule/ecs/components/component.h"
#include "physicsModule/characterMotor.h"
namespace IKIGAI::ECS {
class CharacterMotorComponent : public ComponentBase {
public:
	struct Descriptor : ComponentBase::Descriptor {
		std::string Type;
		ComponentReference Capsule = {};
		ComponentReference Visual = {};
		float StepHeight = .3f;
		float SlopeCos = .7071f;
		float JumpSpeed = 6.f;
		float Gravity = 18.f;
		template <class C> static auto serde(C& c, Descriptor& v) {
			using namespace serde::attribute;
			serde::serde_struct(c, v)
				.field(&Descriptor::Type, "CharacterMotorComponentType")
				.field(&Descriptor::Capsule, "Capsule", default_ { ComponentReference {} })
				.field(&Descriptor::Visual, "Visual", default_ { ComponentReference {} })
				.field(&Descriptor::StepHeight, "StepHeight", default_ { float(.3f) })
				.field(&Descriptor::SlopeCos, "SlopeCos", default_ { float(.7071f) })
				.field(&Descriptor::JumpSpeed, "JumpSpeed", default_ { float(6.f) })
				.field(&Descriptor::Gravity, "Gravity", default_ { float(18.f) });
		}
	};
	explicit CharacterMotorComponent(UTILS::Ref<Object>);
	CharacterMotorComponent(UTILS::Ref<Object>, const ComponentBase::Descriptor& d);
	Descriptor getDescriptor() const {
		return settings;
	}
	static auto GetMembers() {
		return std::tuple {};
	}
	Descriptor settings;
	PHYSICS::CHARACTER::Motor motor;
	bool initialized = false;
	bool jump = false;
	bool sprint = false;
	bool jumpAccepted = false;
	bool landed = false;
	MATH::Vector3f spawn {};
	MATH::Vector3f previous {};
	MATH::Vector3f rendered {};
	MATH::Vector3f wish {};
	float heading = 180;
	float desiredHeading = 180;
	float turnRate = 0;
	float airTime = 0;
	float stepAge = 100;
	float landingAirTime = 0;
	float impactSpeed = 0;
	float remainder = 0;
	float fixedDt = 1.f / 60;
};
template <> inline std::string GetType<CharacterMotorComponent>() {
	return "class IKIGAI::ECS::CharacterMotorComponent";
}
template <> inline std::string GetComponentName<CharacterMotorComponent>() {
	return "CharacterMotorComponent";
}
}
#endif

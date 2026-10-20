#pragma once
#if USE_PHYSICS_3D
#include "component.h"
#include "coreModule/ecs/componentReference.h"
namespace IKIGAI::ECS {
class ThirdPersonController : public ComponentBase {
public:
	struct Descriptor : ComponentBase::Descriptor {
		std::string Type;
		ComponentReference Motor;
		ComponentReference Camera;
		float RunSpeed = 3.5f;
		float SprintSpeed = 6.5f;
		template <class C> static auto serde(C& c, Descriptor& v) {
			using namespace serde::attribute;
			serde::serde_struct(c, v)
				.field(&Descriptor::Type, "ThirdPersonControllerType")
				.field(&Descriptor::Motor, "Motor")
				.field(&Descriptor::Camera, "Camera")
				.field(&Descriptor::RunSpeed, "RunSpeed", default_ { 3.5f })
				.field(&Descriptor::SprintSpeed, "SprintSpeed", default_ { 6.5f });
		}
	};
	explicit ThirdPersonController(UTILS::Ref<Object>);
	ThirdPersonController(UTILS::Ref<Object>, const ComponentBase::Descriptor&);
	Descriptor getDescriptor() const {
		return settings;
	}
	static auto GetMembers() {
		return std::tuple {};
	}
	void readInput(SCENE_SYSTEM::Scene&);
	Descriptor settings;
};
template <> inline std::string GetType<ThirdPersonController>() {
	return "class IKIGAI::ECS::ThirdPersonController";
}
template <> inline std::string GetComponentName<ThirdPersonController>() {
	return "ThirdPersonController";
}
}
#endif

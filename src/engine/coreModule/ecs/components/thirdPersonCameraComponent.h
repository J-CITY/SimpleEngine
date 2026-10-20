#pragma once
#if USE_PHYSICS_3D
#include "coreModule/ecs/componentReference.h"
#include "coreModule/ecs/components/component.h"
namespace IKIGAI::ECS {
class ThirdPersonCameraComponent : public ComponentBase {
public:
	struct Descriptor : ComponentBase::Descriptor {
		std::string Type;
		ComponentReference Camera = {};
		ComponentReference Target = {};
		float Distance = 3.5f;
		float Sensitivity = .12f;
		float PivotHeight = 1.45f;
		template <class C> static auto serde(C& c, Descriptor& v) {
			using namespace serde::attribute;
			serde::serde_struct(c, v)
				.field(&Descriptor::Type, "ThirdPersonCameraComponentType")
				.field(&Descriptor::Camera, "Camera", default_ { ComponentReference {} })
				.field(&Descriptor::Target, "Target", default_ { ComponentReference {} })
				.field(&Descriptor::Distance, "Distance", default_ { float(3.5f) })
				.field(&Descriptor::Sensitivity, "Sensitivity", default_ { float(.12f) })
				.field(&Descriptor::PivotHeight, "PivotHeight", default_ { float(1.45f) });
		}
	};
	explicit ThirdPersonCameraComponent(UTILS::Ref<Object>);
	ThirdPersonCameraComponent(UTILS::Ref<Object>, const ComponentBase::Descriptor& d);
	Descriptor getDescriptor() const {
		return settings;
	}
	static auto GetMembers() {
		return std::tuple {};
	}
	Descriptor settings;
	float yaw = 180;
	float pitch = 15;
	float arm = 3.5f;
	bool captured = false;
	void onDisable() override;
	void onDestroy() override {
		onDisable();
	}
};
template <> inline std::string GetType<ThirdPersonCameraComponent>() {
	return "class IKIGAI::ECS::ThirdPersonCameraComponent";
}
template <> inline std::string GetComponentName<ThirdPersonCameraComponent>() {
	return "ThirdPersonCameraComponent";
}
}
#endif

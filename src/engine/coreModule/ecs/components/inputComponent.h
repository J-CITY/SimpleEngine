#pragma once
#include <functional>
#include <string>

#include "component.h"
#include "utilsModule/reflection/reflection_macros.h"

namespace IKIGAI::ECS { class Object; }

namespace IKIGAI::ECS {
	IKI_CLASS(Groups=[Component])
	class InputComponent : public ComponentBase {
		IKI_GENERATED_BODY(InputComponent)
		bool isActive = true;
		std::function<void(std::chrono::duration<double>)> inputEventFun = [](std::chrono::duration<double>){};
	public:
		IKI_CLASS(Name=InputComponent::Descriptor)
		struct Descriptor : public ComponentBase::Descriptor {
			IKI_GENERATED_BODY(InputComponent::Descriptor)
			IKI_PROPERTY(SEREALIZE(name="InputComponentType"))
			std::string Type;
		};
		InputComponent(UTILS::Ref<ECS::Object> obj, std::function<void(std::chrono::duration<double>)> inputEventFun);
		InputComponent(UTILS::Ref<ECS::Object> obj);
		InputComponent(UTILS::Ref<ECS::Object> obj, const ComponentBase::Descriptor& descriptor) :
			InputComponent(obj) {
		};
		void setActive(bool val);
		bool getActive() const;
		const std::function<void(std::chrono::duration<double>)>& getEventFunc();
		[[nodiscard]] Descriptor getDescriptor() const;
	public:
		static auto GetMembers() {
			return std::tuple{
			};
		}
	};

	template <>
	inline std::string IKIGAI::ECS::GetType<InputComponent>() {
		return "class IKIGAI::ECS::InputComponent";
	}

	template <>
	inline std::string IKIGAI::ECS::GetComponentName<InputComponent>() {
		return "InputComponent";
	}
}

#include "generated/inputComponent.generated.h"

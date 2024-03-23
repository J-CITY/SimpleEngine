#pragma once
#include <functional>
#include <string>

#include "component.h"

namespace IKIGAI::ECS { class Object; }

namespace IKIGAI::ECS {
	class InputComponent : public Component {
		bool isActive = true;
		std::function<void(std::chrono::duration<double>)> inputEventFun = [](std::chrono::duration<double>){};
	public:
		struct Descriptor : public Component::Descriptor {
			std::string Type;

			template<class Context>
			constexpr static auto serde(Context& context, Descriptor& value) {
				using Self = Descriptor;
				using namespace serde::attribute;
				serde::serde_struct(context, value)
					.field(&Self::Type, "InputComponentType");
			}
		};
		InputComponent(UTILS::Ref<ECS::Object> obj, std::function<void(std::chrono::duration<double>)> inputEventFun);
		InputComponent(UTILS::Ref<ECS::Object> obj);
		InputComponent(UTILS::Ref<ECS::Object> obj, const Component::Descriptor& descriptor) :
			InputComponent(obj) {
		};
		void setActive(bool val);
		bool getActive() const;
		const std::function<void(std::chrono::duration<double>)>& getEventFunc();

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

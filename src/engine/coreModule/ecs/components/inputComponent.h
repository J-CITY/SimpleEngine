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
		InputComponent(UTILS::Ref<ECS::Object> obj, std::function<void(std::chrono::duration<double>)> inputEventFun);
		InputComponent(UTILS::Ref<ECS::Object> obj);
		void setActive(bool val);
		bool getActive() const;
		const std::function<void(std::chrono::duration<double>)>& getEventFunc();
	};
}
#pragma once
#include <functional>
#include <string>

#include "component.h"

namespace KUMA::ECS { class Object; }

namespace KUMA::ECS {
	class InputComponent : public Component {
		bool isActive = true;
		std::function<void(std::chrono::duration<double>)> inputEventFun = [](std::chrono::duration<double>){};
	public:
		InputComponent(Ref<ECS::Object> obj, std::function<void(std::chrono::duration<double>)> inputEventFun);
		void setActive(bool val);
		bool getActive() const;
		const std::function<void(std::chrono::duration<double>)>& getEventFunc();

		virtual void onDeserialize(nlohmann::json& j) override {
		}
		virtual void onSerialize(nlohmann::json& j) override {
		}
	};
}
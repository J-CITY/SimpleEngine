#pragma once
#include <functional>
#include <string>

#include "component.h"

namespace KUMA::ECS { class Object; }

namespace KUMA::ECS {
	class InputComponent : public Component {
		bool isActive = true;
		std::function<void(std::chrono::duration<double>)> inputEventFun;
	public:
		InputComponent(Ref<ECS::Object> obj, std::function<void(std::chrono::duration<double>)> inputEventFun);
		void setActive(bool val) {
			isActive = val;
		}
		bool getActive() {
			return isActive;
		}
		void onUpdate(std::chrono::duration<double> dt) override {
			if (isActive && inputEventFun) {
				inputEventFun(dt);
			}
		}
		virtual void onDeserialize(nlohmann::json& j) override {
		}
		virtual void onSerialize(nlohmann::json& j) override {
		}
	};
}
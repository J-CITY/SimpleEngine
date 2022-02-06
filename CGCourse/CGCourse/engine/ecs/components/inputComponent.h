#pragma once
#include <functional>
#include <string>

#include "component.h"

namespace KUMA::ECS { class Object; }

namespace KUMA::ECS {
	class InputComponent : public Component {
		bool isActive = true;
		std::function<void(float)> inputEventFun;
	public:
		InputComponent(const ECS::Object& obj, std::function<void(float)> inputEventFun);
		void setActive(bool val) {
			isActive = val;
		}
		bool getActive() {
			return isActive;
		}
		void onUpdate(float dt) override {
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
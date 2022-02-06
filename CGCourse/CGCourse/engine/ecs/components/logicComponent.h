#pragma once
#include <functional>
#include <string>

#include "component.h"

namespace KUMA::ECS { class Object; }

namespace KUMA::ECS {
	class LogicComponent : public Component {
	public:
		std::function<void()> _onAwake = []{};
		std::function<void()> _onStart = []{};
		std::function<void()> _onEnable = [] {};
		std::function<void()> _onDisable = [] {};
		std::function<void()> _onDestroy = []{};
		std::function<void(float)> _onUpdate = [](float) {};
		std::function<void(float)> _onFixedUpdate = [](float) {};
		std::function<void(float)> _onLateUpdate = [](float) {};
	public:
		LogicComponent(const ECS::Object& obj, std::function<void(float)> inputEventFun);

		virtual void onAwake() override {
			_onAwake();
		}
		virtual void onStart() override {
			_onStart();
		};
		virtual void onEnable() override {
			_onEnable();
		};
		virtual void onDisable() override {
			_onDisable();
		}
		virtual void onDestroy() override {
			_onDestroy();
		};
		virtual void onUpdate(float dt) override {
			_onUpdate(dt);
		};
		virtual void onFixedUpdate(float dt) override {
			_onFixedUpdate(dt);
		};
		virtual void onLateUpdate(float dt) override {
			_onLateUpdate(dt);
		};
		
		virtual void onDeserialize(nlohmann::json& j) override {
		}
		virtual void onSerialize(nlohmann::json& j) override {
		}
	};
}
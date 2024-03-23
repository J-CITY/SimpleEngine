#pragma once
#include <functional>
#include <string>

#include "component.h"

namespace IKIGAI::ECS { class Object; }

namespace IKIGAI::ECS {
	class LogicComponent : public Component {
	public:
		struct Descriptor : public Component::Descriptor {
			std::string Type;

			template<class Context>
			constexpr static auto serde(Context& context, Descriptor& value) {
				using Self = Descriptor;
				using namespace serde::attribute;
				serde::serde_struct(context, value)
					.field(&Self::Type, "LogicComponentType");
			}
		};
		std::function<void()> _onAwake = []{};
		std::function<void()> _onStart = []{};
		std::function<void()> _onEnable = [] {};
		std::function<void()> _onDisable = [] {};
		std::function<void()> _onDestroy = []{};
		std::function<void(std::chrono::duration<double>)> _onUpdate = [](std::chrono::duration<double>) {};
		std::function<void(std::chrono::duration<double>)> _onFixedUpdate = [](std::chrono::duration<double>) {};
		std::function<void(std::chrono::duration<double>)> _onLateUpdate = [](std::chrono::duration<double>) {};
	public:
		LogicComponent(UTILS::Ref<ECS::Object> obj) : Component(obj) {
			__NAME__ = "LogicComponent";
		};
		LogicComponent(UTILS::Ref<ECS::Object> obj, const Component::Descriptor& descriptor) :
			LogicComponent(obj) {
		};

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
		virtual void onUpdate(std::chrono::duration<double> dt) override {
			_onUpdate(dt);
		};
		virtual void onFixedUpdate(std::chrono::duration<double> dt) override {
			_onFixedUpdate(dt);
		};
		virtual void onLateUpdate(std::chrono::duration<double> dt) override {
			_onLateUpdate(dt);
		};
		
		virtual void onDeserialize(nlohmann::json& j) override {
		}
		virtual void onSerialize(nlohmann::json& j) override {
		}

	public:
		static auto GetMembers() {
			return std::tuple{
			};
		}
	};
	template <>
	inline std::string ECS::GetType<LogicComponent>() {
		return "class IKIGAI::ECS::LogicComponent";
	}

	template <>
	inline std::string IKIGAI::ECS::GetComponentName<LogicComponent>() {
		return "LogicComponent";
	}
}

#pragma once

#include <sol/sol.hpp>

#include "component.h"
#include "../../utils/event.h"

import logger;

namespace KUMA::ECS { class Object; }

namespace KUMA::ECS {
	class ScriptComponent : public Component {
	public:
		ScriptComponent(const ECS::Object& obj, const std::string& name);

		~ScriptComponent();
		bool registerToLuaContext(sol::state& luaState, const std::string& scriptFolder);
		void unregisterFromLuaContext();
		template<typename... Args>
		void luaCall(const std::string& p_functionName, Args&&... p_args);
		sol::table& getTable();
		virtual void onAwake() override;
		virtual void onStart() override;
		virtual void onEnable() override;
		virtual void onDisable() override;
		virtual void onDestroy() override;
		virtual void onUpdate(std::chrono::duration<double> dt) override;
		virtual void onFixedUpdate(std::chrono::duration<double> dt) override;
		virtual void onLateUpdate(std::chrono::duration<double> dt) override;

		static KUMA::EVENT::Event<std::shared_ptr<KUMA::ECS::ScriptComponent>> createdEvent;
		static KUMA::EVENT::Event<std::shared_ptr<KUMA::ECS::ScriptComponent>> destroyedEvent;

		virtual void onDeserialize(nlohmann::json& j) override {
			name = j["data"]["name"];
		}
		virtual void onSerialize(nlohmann::json& j) override {
			j["data"]["name"] = name;
		}
		std::string getName() const;
	private:
		std::string name;
		sol::table object = sol::nil;
	};

	template<typename ...Args>
	inline void ECS::ScriptComponent::luaCall(const std::string& functionName, Args&& ...args) {
		if (object.valid()) {
			if (object[functionName].valid()) {
				sol::protected_function pfr = object[functionName];
				auto pfrResult = pfr.call(object, std::forward<Args>(args)...);
				if (!pfrResult.valid()) {
					sol::error err = pfrResult;
					LOG_ERROR(err.what());
				}
			}
		}
	}
}

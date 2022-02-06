#pragma once

#include <sol/sol.hpp>

#include "component.h"
#include "../../utils/debug/logger.h"
#include "../../utils/event.h"

namespace KUMA::ECS { class Object; }

namespace KUMA::ECS {
	class Script : public Component {
	public:
		Script(const ECS::Object& obj, const std::string& name);

		~Script();
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
		virtual void onUpdate(float dt) override;
		virtual void onFixedUpdate(float dt) override;
		virtual void onLateUpdate(float dt) override;

		static KUMA::EVENT::Event<Script*> createdEvent;
		static KUMA::EVENT::Event<Script*> destroyedEvent;

		virtual void onDeserialize(nlohmann::json& j) override {
			name = j["data"]["name"];
		}
		virtual void onSerialize(nlohmann::json& j) override {
			j["data"]["name"] = name;
		}
	public:
		std::string name;

	private:
		sol::table object = sol::nil;
	};

	template<typename ...Args>
	inline void ECS::Script::luaCall(const std::string& functionName, Args&& ...args) {
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

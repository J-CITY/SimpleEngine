#pragma once
#include "../systemManager.h"
#include "../components/scriptComponent.h"

namespace KUMA::ECS {
	class Object;
}

namespace KUMA::ECS {
	class ScriptSystem : public System {
	public:
		ScriptSystem();
		void onAwake() override;
		void onStart() override;
		void onEnable() override;
		void onDisable() override;
		void onDestroy() override;
		void onUpdate(std::chrono::duration<double> dt) override;
		void onFixedUpdate(std::chrono::duration<double> dt) override;
		void onLateUpdate(std::chrono::duration<double> dt) override;

		bool registerToLuaContext(ScriptComponent& component, sol::state& luaState, const std::string& scriptFolder);
		void unregisterFromLuaContext(ScriptComponent& component);
	private:
		template<typename ...Args>
		inline void luaCall(ScriptComponent& component, const std::string& functionName, Args&& ...args) {
			auto& object = component.getTable();
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
	};
}
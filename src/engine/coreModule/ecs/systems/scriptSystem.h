#pragma once
#include "../systemManager.h"
#include "../components/scriptComponent.h"
#include "ecsModule/systemManager.h"
#include "utilsModule/log/loggerDefine.h"

namespace IKIGAI::ECS {
	class Object;
}

namespace IKIGAI::ECS {
	class ScriptSystem : public IKIGAI::ECS2::System {
	public:
		ScriptSystem();
		void onAwake(ECS2::World& world) override;
		void onStart(ECS2::World& world) override;
		void onEnable(ECS2::World& world) override;
		void onDisable(ECS2::World& world) override;
		void onDestroy(ECS2::World& world) override;
		void onUpdate(ECS2::World& world, ECS2::CommandBuffer& cb, std::chrono::duration<double> dt) override;
		void onFixedUpdate(ECS2::World& world, ECS2::CommandBuffer& cb, std::chrono::duration<double> dt) override;
		void onLateUpdate(ECS2::World& world, ECS2::CommandBuffer& cb, std::chrono::duration<double> dt) override;

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
						LOG_ERROR << (err.what());
					}
				}
			}
		}
	};
}
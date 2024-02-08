#pragma once

#include <memory>
#include <string>
#include <vector>
#include "utilsModule/weakPtr.h"
#include "coreModule/ecs/components/scriptComponent.h"

namespace sol {
	class state;
}


namespace IKIGAI::SCRIPTING {
	class ScriptInterpreter {
	public:
		ScriptInterpreter(const std::string& scriptRootFolder);
		~ScriptInterpreter();

		void createLuaContextAndBindGlobals();
		void destroyLuaContext();
		void consider(UTILS::WeakPtr<IKIGAI::ECS::ScriptComponent> toConsider);
		void unconsider(UTILS::WeakPtr<IKIGAI::ECS::ScriptComponent> toUnconsider);
		void refreshAll();
		bool isOk() const;
	private:
		std::unique_ptr<sol::state> luaState;
		std::string scriptRootFolder;
		std::vector<UTILS::WeakPtr<IKIGAI::ECS::ScriptComponent>> scripts;
		bool checkOk;
	};
}

#pragma once

#include <memory>
#include <vector>

#include <sol/sol.hpp>

#include "../ecs/components/script.h"

namespace KUMA::SCRIPTING {
	class ScriptInterpreter {
	public:
		ScriptInterpreter(const std::string& scriptRootFolder);
		~ScriptInterpreter();

		void createLuaContextAndBindGlobals();
		void destroyLuaContext();
		void consider(KUMA::ECS::Script* toConsider);
		void unconsider(KUMA::ECS::Script* toUnconsider);
		void refreshAll();
		bool isOk() const;

	private:
		std::unique_ptr<sol::state> luaState;
		std::string scriptRootFolder;
		std::vector<KUMA::ECS::Script*> scripts;
		bool checkOk;
	};
}
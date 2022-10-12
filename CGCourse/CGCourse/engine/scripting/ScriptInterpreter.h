#pragma once

#include <memory>
#include <string>
#include <vector>
#include "../utils/pointers/objPtr.h"

namespace sol {
	class state;
}

namespace KUMA::ECS {
	class ScriptComponent;
}

namespace KUMA::SCRIPTING {
	class ScriptInterpreter {
	public:
		ScriptInterpreter(const std::string& scriptRootFolder);
		~ScriptInterpreter();

		void createLuaContextAndBindGlobals();
		void destroyLuaContext();
		void consider(object_ptr<KUMA::ECS::ScriptComponent> toConsider);
		void unconsider(object_ptr<KUMA::ECS::ScriptComponent> toUnconsider);
		void refreshAll();
		bool isOk() const;
	private:
		std::unique_ptr<sol::state> luaState;
		std::string scriptRootFolder;
		std::vector<object_ptr<KUMA::ECS::ScriptComponent>> scripts;
		bool checkOk;
	};
}

#pragma once

#include <memory>
#include <string>
#include <vector>
#include <utilsModule/pointers/objPtr.h>

namespace sol {
	class state;
}

namespace IKIGAI::ECS {
	class ScriptComponent;
}

namespace IKIGAI::SCRIPTING {
	class ScriptInterpreter {
	public:
		ScriptInterpreter(const std::string& scriptRootFolder);
		~ScriptInterpreter();

		void createLuaContextAndBindGlobals();
		void destroyLuaContext();
		void consider(object_ptr<IKIGAI::ECS::ScriptComponent> toConsider);
		void unconsider(object_ptr<IKIGAI::ECS::ScriptComponent> toUnconsider);
		void refreshAll();
		bool isOk() const;
	private:
		std::unique_ptr<sol::state> luaState;
		std::string scriptRootFolder;
		std::vector<object_ptr<IKIGAI::ECS::ScriptComponent>> scripts;
		bool checkOk;
	};
}

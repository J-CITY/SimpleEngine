#pragma once

#include <memory>
#include <string>
#include <vector>

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
		void consider(std::shared_ptr<KUMA::ECS::ScriptComponent> toConsider);
		void unconsider(std::shared_ptr<KUMA::ECS::ScriptComponent> toUnconsider);
		void refreshAll();
		bool isOk() const;
	private:
		std::unique_ptr<sol::state> luaState;
		std::string scriptRootFolder;
		std::vector<std::shared_ptr<KUMA::ECS::ScriptComponent>> scripts;
		bool checkOk;
	};
}

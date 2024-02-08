#pragma once

#include <sol/sol.hpp>

#include "component.h"
#include <utilsModule/event.h>
#include <rttr/registration_friend>

import logger;

namespace IKIGAI::ECS { class Object; }

namespace IKIGAI::ECS {
	class ScriptComponent : public Component {
	private:
		RTTR_REGISTRATION_FRIEND
	public:
		ScriptComponent(UTILS::Ref<ECS::Object> obj);
		ScriptComponent(UTILS::Ref<ECS::Object> obj, const std::string& name);
		~ScriptComponent() override;
		sol::table& getTable();

		[[nodiscard]] const std::string& getName() const;
	private:
		void setScript(std::string name);
		std::string getScriptName();

		std::string name;
		sol::table object = sol::nil;
	};

	struct ScriptComponentEvents {
		static IKIGAI::EVENT::Event<UTILS::WeakPtr<IKIGAI::ECS::ScriptComponent>> createdEvent;
		static IKIGAI::EVENT::Event<UTILS::WeakPtr<IKIGAI::ECS::ScriptComponent>> destroyedEvent;
	};
}

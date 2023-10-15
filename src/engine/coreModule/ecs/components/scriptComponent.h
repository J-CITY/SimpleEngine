#pragma once

#include <sol/sol.hpp>

#include "component.h"
#include <utilsModule/event.h>
#include <utilsModule/pointers/objPtr.h>
#include <rttr/registration_friend>

import logger;

namespace IKIGAI::ECS { class Object; }

namespace IKIGAI::ECS {
	class ScriptComponent : public Component {
	private:
		RTTR_REGISTRATION_FRIEND
	public:
		ScriptComponent(Ref<ECS::Object> obj);
		ScriptComponent(Ref<ECS::Object> obj, const std::string& name);
		~ScriptComponent() override;
		sol::table& getTable();

		static IKIGAI::EVENT::Event<object_ptr<IKIGAI::ECS::ScriptComponent>> createdEvent;
		static IKIGAI::EVENT::Event<object_ptr<IKIGAI::ECS::ScriptComponent>> destroyedEvent;

		[[nodiscard]] const std::string& getName() const;
	private:
		void setScript(std::string name);
		std::string getScriptName();

		std::string name;
		sol::table object = sol::nil;
	};
}

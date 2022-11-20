#pragma once

#include <sol/sol.hpp>

#include "component.h"
#include "../../utils/event.h"
#include "../../utils/pointers/objPtr.h"

import logger;

namespace KUMA::ECS { class Object; }

namespace KUMA::ECS {
	class ScriptComponent : public Component {
	public:
		ScriptComponent(Ref<ECS::Object> obj, const std::string& name);
		~ScriptComponent() override;
		sol::table& getTable();

		static KUMA::EVENT::Event<object_ptr<KUMA::ECS::ScriptComponent>> createdEvent;
		static KUMA::EVENT::Event<object_ptr<KUMA::ECS::ScriptComponent>> destroyedEvent;

		virtual void onDeserialize(nlohmann::json& j) override {
			name = j["data"]["name"];
		}
		virtual void onSerialize(nlohmann::json& j) override {
			j["data"]["name"] = name;
		}

		[[nodiscard]] std::string getName() const;
	private:
		std::string name;
		sol::table object = sol::nil;
	};
}

#pragma once

#include <sol/sol.hpp>

#include "component.h"
#include <utilsModule/event.h>

namespace IKIGAI::ECS { class Object; }

namespace IKIGAI::ECS {
	class ScriptComponent : public Component {
	public:
		struct Descriptor : public Component::Descriptor {
			std::string Type;
			std::string Path;

			template<class Context>
			constexpr static auto serde(Context& context, Descriptor& value) {
				using Self = Descriptor;
				using namespace serde::attribute;
				serde::serde_struct(context, value)
					.field(&Self::Type, "ScriptComponentType")
					.field(&Self::Path, "Path");
			}
		};
		ScriptComponent(UTILS::Ref<ECS::Object> obj);
		ScriptComponent(UTILS::Ref<ECS::Object> obj, const std::string& name);
		ScriptComponent(UTILS::Ref<ECS::Object> obj, const Descriptor& name);
		ScriptComponent(UTILS::Ref<ECS::Object> obj, const Component::Descriptor& descriptor) :
			ScriptComponent(obj, static_cast<const Descriptor&>(descriptor)) {
		};
		~ScriptComponent() override;
		sol::table& getTable();

		[[nodiscard]] const std::string& getName() const;
	private:
		void setScript(std::string name);
		std::string getScriptName();

		std::string name;
		sol::table object = sol::nil;
	public:
		static auto GetMembers() {
			return std::tuple{
			};
		}
	};

	struct ScriptComponentEvents {
		static IKIGAI::EVENT::Event<UTILS::WeakPtr<IKIGAI::ECS::ScriptComponent>> createdEvent;
		static IKIGAI::EVENT::Event<UTILS::WeakPtr<IKIGAI::ECS::ScriptComponent>> destroyedEvent;
	};

	template <>
	inline std::string ECS::GetType<ScriptComponent>() {
		return "class IKIGAI::ECS::ScriptComponent";
	}

	template <>
	inline std::string IKIGAI::ECS::GetComponentName<ScriptComponent>() {
		return "ScriptComponent";
	}
}

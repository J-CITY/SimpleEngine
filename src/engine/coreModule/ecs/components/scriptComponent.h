#pragma once

#include <sol/sol.hpp>

#include "component.h"
#include <utilsModule/event.h>
#include "utilsModule/reflection/reflection_macros.h"

namespace IKIGAI::ECS { class Object; }

namespace IKIGAI::ECS {
	IKI_CLASS()
	class ScriptComponent : public Component {
		IKI_GENERATED_BODY(ScriptComponent)
	public:
		IKI_CLASS(Name=ScriptComponent::Descriptor)
		struct Descriptor : public Component::Descriptor {
			IKI_GENERATED_BODY(ScriptComponent::Descriptor)
			IKI_PROPERTY(SEREALIZE(name="ScriptComponentType"))
			std::string Type;
			IKI_PROPERTY(SEREALIZE(name="Path"))
			std::string Path;
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

		[[nodiscard]] Descriptor getDescriptor() const;
	private:
		void setScript(std::string name);
		std::string getScriptName() const;

		std::string name;
		sol::table object = sol::nil;
	public:
		IKI_PROPERTY(Name=Path, Type=std::string, Getter=getScriptName, Setter=setScript, Flags=[USE_IN_EDITOR_COMPONENT_INSPECTOR], EditWidget=STRING)
		static auto GetMembers() {
			return std::tuple{
				IKIGAI::UTILS::MakeMemberInfo("Path", &ScriptComponent::getScriptName, &ScriptComponent::setScript,
				UTILS::Meta_t{
					{UTILS::MetaParam::FLAGS, UTILS::MetaInfo::USE_IN_EDITOR_COMPONENT_INSPECTOR},
				{UTILS::MetaParam::EDIT_WIDGET, UTILS::WidgetType::STRING},
				})
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

#include "generated/scriptComponent.generated.h"

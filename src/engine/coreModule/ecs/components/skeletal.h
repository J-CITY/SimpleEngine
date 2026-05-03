#pragma once
#include <optional>

#include "component.h"
#include "utilsModule/reflection/reflection_macros.h"


namespace IKIGAI::RESOURCES {
	class Animator;
	class Animation;
}

namespace IKIGAI::ECS { class Object; }

namespace IKIGAI::ECS {
	IKI_CLASS()
	class Skeletal : public Component {
		IKI_GENERATED_BODY(Skeletal)
	public:
		IKI_CLASS(Name=Skeletal::Descriptor)
		struct Descriptor : public Component::Descriptor {
			IKI_GENERATED_BODY(Skeletal::Descriptor)
			IKI_PROPERTY(SEREALIZE(name="SkeletalType"))
			std::string Type;
			IKI_PROPERTY(SEREALIZE(name="Path"))
			std::string Path;
			IKI_PROPERTY(SEREALIZE(name="Animation"))
			std::string Animation;
		};
		Skeletal(UTILS::Ref<ECS::Object> _obj);
		Skeletal(UTILS::Ref<ECS::Object> _obj, const std::string& _path, const std::optional<std::string>& _startAnimation=std::nullopt);
		Skeletal(UTILS::Ref<ECS::Object> _obj, const Descriptor& _descriptor);
		Skeletal(UTILS::Ref<ECS::Object> obj, const Component::Descriptor& descriptor) :
			Skeletal(obj, static_cast<const Descriptor&>(descriptor)) {
		};
		void onUpdate(std::chrono::duration<double> dt) override;
		void setAnimation(std::string id);

		void setAnimationPath(std::string id);
		std::string getAnimationPath() const;
		//void setAnimation(int id);

		std::string getCurrentAnimationName() const;

		[[nodiscard]] Descriptor getDescriptor() const;

		std::map<std::string, std::shared_ptr<IKIGAI::RESOURCES::Animation>> animations;
		std::shared_ptr<IKIGAI::RESOURCES::Animator> animator;
		std::string animationPath;
		std::optional<std::string> curAnimation;

		float pointX = 0.0f;
		float pointY = 0.0f;
		float getx() const { return pointX; }
		float gety() const { return pointY; }
		void setx(float v) { pointX = v; }
		void sety(float v) { pointY = v; }
	public:
		IKI_PROPERTY(Name=AnimationPath, Type=std::string, Getter=getAnimationPath, Setter=setAnimationPath, Flags=[USE_IN_EDITOR_COMPONENT_INSPECTOR], EditWidget=STRING)
		IKI_PROPERTY(Name=Animation, Type=std::string, Getter=getCurrentAnimationName, Setter=setAnimation, Flags=[USE_IN_EDITOR_COMPONENT_INSPECTOR], EditWidget=STRING)
		IKI_PROPERTY(Name=PointX, Type=float, Getter=getx, Setter=setx, Flags=[USE_IN_EDITOR_COMPONENT_INSPECTOR], EditWidget=DRAG_FLOAT)
		IKI_PROPERTY(Name=PointY, Type=float, Getter=gety, Setter=sety, Flags=[USE_IN_EDITOR_COMPONENT_INSPECTOR], EditWidget=DRAG_FLOAT)
		static auto GetMembers() {
			return std::tuple{
				IKIGAI::UTILS::MakeMemberInfo("AnimationPath", &Skeletal::getAnimationPath, &Skeletal::setAnimationPath,
				UTILS::Meta_t{
					{UTILS::MetaParam::FLAGS, UTILS::MetaInfo::USE_IN_EDITOR_COMPONENT_INSPECTOR},
				{UTILS::MetaParam::EDIT_WIDGET, UTILS::WidgetType::STRING},
				}),
				IKIGAI::UTILS::MakeMemberInfo("Animation", &Skeletal::getCurrentAnimationName, &Skeletal::setAnimation,
				UTILS::Meta_t{
					{UTILS::MetaParam::FLAGS, UTILS::MetaInfo::USE_IN_EDITOR_COMPONENT_INSPECTOR},
				{UTILS::MetaParam::EDIT_WIDGET, UTILS::WidgetType::STRING},
				}),
				IKIGAI::UTILS::MakeMemberInfo("PointX", &Skeletal::getx, &Skeletal::setx,
				UTILS::Meta_t{
					{UTILS::MetaParam::FLAGS, UTILS::MetaInfo::USE_IN_EDITOR_COMPONENT_INSPECTOR},
				{UTILS::MetaParam::EDIT_WIDGET, UTILS::WidgetType::DRAG_FLOAT},
				}),
				IKIGAI::UTILS::MakeMemberInfo("PointY", &Skeletal::gety, &Skeletal::sety,
				UTILS::Meta_t{
					{UTILS::MetaParam::FLAGS, UTILS::MetaInfo::USE_IN_EDITOR_COMPONENT_INSPECTOR},
				{UTILS::MetaParam::EDIT_WIDGET, UTILS::WidgetType::DRAG_FLOAT},
				})
			};
		}
	};

	template <>
	inline std::string ECS::GetType<Skeletal>() {
		return "class IKIGAI::ECS::Skeletal";
	}

	template <>
	inline std::string IKIGAI::ECS::GetComponentName<Skeletal>() {
		return "Skeletal";
	}
}

#include "generated/skeletal.generated.h"

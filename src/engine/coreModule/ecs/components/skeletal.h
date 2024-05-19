#pragma once
#include <optional>

#include "component.h"


namespace IKIGAI::RESOURCES {
	class Animator;
	class Animation;
}

namespace IKIGAI::ECS { class Object; }

namespace IKIGAI::ECS {
	class Skeletal : public Component {
	public:
		struct Descriptor : public Component::Descriptor {
			std::string Type;
			std::string Path;
			std::string Animation;

			template<class Context>
			constexpr static auto serde(Context& context, Descriptor& value) {
				using Self = Descriptor;
				using namespace serde::attribute;
				serde::serde_struct(context, value)
					.field(&Self::Type, "SkeletalType")
					.field(&Self::Path, "Path")
					.field(&Self::Animation, "Animation");
			}
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

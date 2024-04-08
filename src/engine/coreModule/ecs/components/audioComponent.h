#pragma once
#include "component.h"
#include <audioModule/audioManager.h>

#include "pointLight.h"


namespace IKIGAI::ECS { class Object; }

namespace IKIGAI::ECS {
	class AudioComponent : public Component {
	public:
		struct Descriptor : public Component::Descriptor {
			std::string Type;
			std::string AudioPath;

			template<class Context>
			constexpr static auto serde(Context& context, Descriptor& value) {
				using Self = Descriptor;
				using namespace serde::attribute;
				serde::serde_struct(context, value)
					.field(&Self::Type, "AudioComponentType")
					.field(&Self::AudioPath, "AudioPath");
			}
		};
		AudioComponent(UTILS::Ref<ECS::Object> _obj);

		AudioComponent(UTILS::Ref<ECS::Object> _obj, std::string& path);
		AudioComponent(UTILS::Ref<ECS::Object> _obj, const Descriptor& descriptor);
		AudioComponent(UTILS::Ref<ECS::Object> obj, const Component::Descriptor& descriptor) :
			AudioComponent(obj, static_cast<const Descriptor&>(descriptor)) {
		};

		std::string getSourcePath() const;
		void setSourcePath(std::string val);

		std::string getSoundPath() const;
		void setSoundPath(std::string);

		bool getIs3D() const;
		void setIs3D(bool val);

		bool getIsLooped() const;
		void setIsLooped(bool val);

		float getVolume() const;
		void setVolume(float val);

		float getPan() const;
		void setPan(float val);

		AUDIO::SoundStatus getState() const;
		void setState(AUDIO::SoundStatus val);

		std::shared_ptr<AUDIO::SoundResource> res;

		[[nodiscard]] Descriptor getDescriptor() const;
	public:
		static auto GetMembers() {
			return std::tuple{
				IKIGAI::UTILS::MakeMemberInfo("SourcePath", &AudioComponent::getSourcePath, &AudioComponent::setSourcePath,
				UTILS::Meta_t{
					{UTILS::MetaParam::FLAGS, UTILS::MetaInfo::USE_IN_EDITOR_COMPONENT_INSPECTOR},
				{UTILS::MetaParam::EDIT_WIDGET, UTILS::WidgetType::STRING},
				}),
				IKIGAI::UTILS::MakeMemberInfo("Sound", &AudioComponent::getSoundPath, &AudioComponent::setSoundPath,
				UTILS::Meta_t{
					{UTILS::MetaParam::FLAGS, UTILS::MetaInfo::USE_IN_EDITOR_COMPONENT_INSPECTOR},
				{UTILS::MetaParam::EDIT_WIDGET, UTILS::WidgetType::STRING},
				}),
				IKIGAI::UTILS::MakeMemberInfo("Is 3D", &AudioComponent::getIs3D, &AudioComponent::setIs3D,
				UTILS::Meta_t{
					{UTILS::MetaParam::FLAGS, UTILS::MetaInfo::USE_IN_EDITOR_COMPONENT_INSPECTOR},
				{UTILS::MetaParam::EDIT_WIDGET, UTILS::WidgetType::BOOL},
				}),
				IKIGAI::UTILS::MakeMemberInfo("Volume", &AudioComponent::getVolume, &AudioComponent::setVolume,
				UTILS::Meta_t{
					{UTILS::MetaParam::FLAGS, UTILS::MetaInfo::USE_IN_EDITOR_COMPONENT_INSPECTOR},
				{UTILS::MetaParam::EDIT_RANGE, MATH::Vector2f{0.0f, 1.0f}},
				{UTILS::MetaParam::EDIT_STEP, 0.01f},
				{UTILS::MetaParam::EDIT_WIDGET, UTILS::WidgetType::DRAG_FLOAT},
				}),
				IKIGAI::UTILS::MakeMemberInfo("Pan", &AudioComponent::getPan, &AudioComponent::setPan,
				UTILS::Meta_t{
					{UTILS::MetaParam::FLAGS, UTILS::MetaInfo::USE_IN_EDITOR_COMPONENT_INSPECTOR},
				{UTILS::MetaParam::EDIT_RANGE, MATH::Vector2f{0.0f, 100.0f}},
				{UTILS::MetaParam::EDIT_STEP, 1.0f},
				{UTILS::MetaParam::EDIT_WIDGET, UTILS::WidgetType::DRAG_FLOAT},
				}),
				IKIGAI::UTILS::MakeMemberInfo("State", &AudioComponent::getState, &AudioComponent::setState,
				UTILS::Meta_t{
					{UTILS::MetaParam::FLAGS, UTILS::MetaInfo::USE_IN_EDITOR_COMPONENT_INSPECTOR},
				{UTILS::MetaParam::EDIT_WIDGET, UTILS::WidgetType::COMBO},
				})
			};
		}
	};

	class AudioListenerComponent : public Component {
	public:
		struct Descriptor : public Component::Descriptor {
			std::string Type;

			template<class Context>
			constexpr static auto serde(Context& context, Descriptor& value) {
				using Self = Descriptor;
				using namespace serde::attribute;
				serde::serde_struct(context, value)
					.field(&Self::Type, "AudioListenerComponentType");
			}
		};
		AudioListenerComponent(UTILS::Ref<ECS::Object> obj);
		AudioListenerComponent(UTILS::Ref<ECS::Object> obj, const Component::Descriptor& descriptor) :
			AudioListenerComponent(obj) {
		};

		[[nodiscard]] Descriptor getDescriptor() const;
	public:
		static auto GetMembers() {
			return std::tuple{
			};
		}
	};

	template <>
	inline	std::string ECS::GetType<AudioListenerComponent>() {
		return "class IKIGAI::ECS::AudioListenerComponent";
	}
	template <>
	inline std::string ECS::GetType<AudioComponent>() {
		return "class IKIGAI::ECS::AudioComponent";
	}

	template <>
	inline std::string IKIGAI::ECS::GetComponentName<AudioListenerComponent>() {
		return "AudioListenerComponent";
	}
	template <>
	inline std::string IKIGAI::ECS::GetComponentName<AudioComponent>() {
		return "AudioComponent";
	}
}

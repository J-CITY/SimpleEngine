#pragma once
#include "component.h"
#include <audioModule/audioManager.h>


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

		std::string getSourcePath();
		void setSourcePath(std::string val);

		std::string getSoundPath();
		void setSoundPath(std::string);

		bool getIs3D();
		void setIs3D(bool val);

		bool getIsLooped();
		void setIsLooped(bool val);

		float getVolume();
		void setVolume(float val);

		float getPan();
		void setPan(float val);

		AUDIO::SoundStatus getState();
		void setState(AUDIO::SoundStatus val);

		std::shared_ptr<AUDIO::SoundResource> res;

	public:
		static auto GetMembers() {
			return std::tuple{
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

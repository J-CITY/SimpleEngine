#pragma once
#include "component.h"
#include <audioModule/audioManager.h>


namespace IKIGAI::ECS { class Object; }

namespace IKIGAI::ECS {
	class AudioComponent : public Component {
	public:
		AudioComponent(UTILS::Ref<ECS::Object> obj);

		AudioComponent(UTILS::Ref<ECS::Object> obj, std::string& path);

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
	};

	class AudioListenerComponent : public Component {
	public:
		AudioListenerComponent(UTILS::Ref<ECS::Object> obj);
	};
}
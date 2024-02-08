#pragma once
#include <memory>
#include <optional>
#include <string>
#include <rttr/rttr_enable.h>

#include "soloud.h"

namespace IKIGAI::AUDIO {
	class AudioMixerSource;

	enum class SoundType {
		SOUND, MUSIC, BACKGROUND
	};

	enum class SoundStatus {
		PLAY, PAUSE, STOP
	};

	struct SoundConfig {
		SoundType type = SoundType::SOUND;
		float volume = -1.0f;
		SoundStatus state = SoundStatus::STOP;
		float pan = 0.0f;
		//float panFadeTime = 0.0f;
		//float fadeTime = 0.0f;
		bool is3D = false;
		bool isLooped = false;
		std::string pathSoundSource;

		RTTR_ENABLE()
	};

	class SoundResource {
	public:
		
		SoundResource(const std::string& path, SoundConfig& data) : path(path), data(data) {}

		unsigned int getBusId();

	//private:
		std::optional<SoLoud::handle> id;
		std::string path;
	public:
		SoundConfig data;
		std::shared_ptr<AudioMixerSource> mixer;
	};
}

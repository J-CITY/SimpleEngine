#pragma once

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include "soloud.h"

namespace IKIGAI::AUDIO {
	class SoundResource;

	enum class MixerStatus {
		PLAY, PAUSE, STOP
	};

	struct MixerConfig {
		float volume = -1.0f;
		MixerStatus state = MixerStatus::STOP;
		float pan = 0.0f;
	};

	class AudioMixerSource {
		std::unordered_map<std::string, MixerConfig> presets;

		std::string current;

		std::unordered_set<std::shared_ptr<SoundResource>> soundResources;

		SoLoud::Bus bus;
	public:
		int bushandle = 0;
		AudioMixerSource();

		void setPreset(const std::string& preset);
		void stop();
		int getIdBus() {
			return bushandle;
		}

		//TODO:
		//add filters
		//add presets for each audio source for each layer
		//add map to audio manager for get AudioMixerSource on call play function ...
	};
}


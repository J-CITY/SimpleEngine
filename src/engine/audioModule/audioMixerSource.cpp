#include "audioMixerSource.h"

#include "audioManager.h"

using namespace IKIGAI;
using namespace IKIGAI::AUDIO;


AudioMixerSource::AudioMixerSource() {
	bushandle = AudioManagerAL::GetInstance().playBus(bus);
}

void AudioMixerSource::setPreset(const std::string& preset) {
	if (current == preset) {
		return;
	}
	if (!presets.contains(preset)) {
		//problem
		return;
	}
	current = preset;
	auto& config = presets.at(current);

	auto& audioManager = AudioManagerAL::GetInstance();

	audioManager.setVolume(*this, config.volume);
	audioManager.setPan(*this, config.pan);
}

void AudioMixerSource::stop() {
	bus.stop();
}

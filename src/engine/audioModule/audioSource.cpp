#include "audioSource.h"

#include "audioMixerSource.h"

using namespace IKIGAI;
using namespace IKIGAI::AUDIO;

unsigned SoundResource::getBusId() {
	return mixer ? mixer->getIdBus() : 0;
}

#include <rttr/registration>

RTTR_REGISTRATION
{
	rttr::registration::class_<IKIGAI::AUDIO::SoundConfig>("SoundConfig")
	.property("type", &IKIGAI::AUDIO::SoundConfig::type)
	.property("volume", &IKIGAI::AUDIO::SoundConfig::volume)
	.property("state", &IKIGAI::AUDIO::SoundConfig::state)
	.property("pan", &IKIGAI::AUDIO::SoundConfig::pan)
	.property("is3D", &IKIGAI::AUDIO::SoundConfig::is3D)
	.property("isLooped", &IKIGAI::AUDIO::SoundConfig::isLooped)
	.property("pathSoundSource", &IKIGAI::AUDIO::SoundConfig::pathSoundSource);

	rttr::registration::enumeration<IKIGAI::AUDIO::SoundType>("SoundType")
	(
		rttr::value("SOUND", SoundType::SOUND),
		rttr::value("MUSIC", SoundType::MUSIC),
		rttr::value("BACKGROUND", SoundType::BACKGROUND)
	);

	rttr::registration::enumeration<IKIGAI::AUDIO::SoundStatus>("SoundType")
	(
		rttr::value("PLAY", SoundStatus::PLAY),
		rttr::value("PAUSE", SoundStatus::PAUSE),
		rttr::value("STOP", SoundStatus::STOP)
	);
}


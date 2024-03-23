#include "audioSource.h"

#include "audioMixerSource.h"

using namespace IKIGAI;
using namespace IKIGAI::AUDIO;

unsigned SoundResource::getBusId() {
	return mixer ? mixer->getIdBus() : 0;
}



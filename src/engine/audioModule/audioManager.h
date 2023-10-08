
#pragma once

#include <filesystem>
#include <string>
//#include <irrKlang.h>
import glmath;

#include <unordered_set>

#include "audioMixerSource.h"

#define NOMINMAX
#include "soloud.h"
#include "soloud_wav.h"
#include "soloud_wavstream.h"
#include "audioSource.h"
#include "utilsModule/singleton.h"

namespace IKIGAI {
	namespace AUDIO
	{
		struct Sound2d {
			std::string path;
		};

		struct Sound3d: public Sound2d {
			MATHGL::Vector3 pos;
		};

		class AudioManager
		{
			//irrklang::ISoundEngine* soundEngine;
		public:
			AudioManager() {
				//soundEngine = irrklang::createIrrKlangDevice();
			}

			~AudioManager() {
				//soundEngine->drop();
			}

			void play2d(Sound2d sound, bool isLooped) {
				//auto soundTracker = soundEngine->play2D(sound.path.c_str(), isLooped);
			}

			void play3d(Sound3d sound, bool isLooped) {
				//auto soundTracker = soundEngine->play3D(sound.path.c_str(), irrklang::vec3df(sound.pos.x, sound.pos.y, sound.pos.z), isLooped);
			}

			void setListenerPos(MATHGL::Vector3 pos, MATHGL::Vector3 dir) {
				//soundEngine->setListenerPosition(irrklang::vec3df(pos.x, pos.y, pos.z), irrklang::vec3df(dir.x, dir.y, dir.z));
			}

			void pauseAll() {

			}

			void unpauseAll() {

			}

			void pause() {
				
			}

			void unpause() {

			}

		};


		//TODO:
		//Add component whitch call audio system



		class AudioManagerAL : public UTILS::SingletonService<AudioManagerAL> {
			FRIEND_SINGLETON_SERVICE(AudioManagerAL)

			std::unique_ptr<SoLoud::Soloud> audioEngine;

			std::unordered_map<std::string, SoLoud::Wav> sounds;
			std::unordered_map<std::string, SoLoud::WavStream> musics;
			AudioManagerAL();

		public:
			~AudioManagerAL() override;

			unsigned int playBus(SoLoud::Bus& bus);

			void addSource(SoundResource& source);
			void removeSource(std::optional<SoLoud::handle> id);

			void set3dSoundSpeedSpeed(float speed);
			void setListenerPos(MATHGL::Vector3 pos, MATHGL::Vector3 dir);

			void setListenerVelocity(MATHGL::Vector3 vel);
			void setListenerUp(MATHGL::Vector3 up);
			void setSourcePos(SoundResource& source, MATHGL::Vector3 pos);
			void setSourceVelocity(SoundResource& source, MATHGL::Vector3 vel);
			void setSourceMinMaxDistance(SoundResource& source, float minDistance, float maxDistance);
			void setSourceAttenuation(SoundResource& source, unsigned int model, float factor);
			void setSourceDopplerFactor(SoundResource& source, float factor);

			void pauseAll();
			void unpauseAll();

			void stopAll();
			void stop(SoundResource& source);
			void pause(SoundResource& source);
			void play(SoundResource& source);
			void unpause(SoundResource& source);
			void setVolume(SoundResource& source, float vol);
			void setVolume(AudioMixerSource& source, float vol);
			void setGlobalVolume(float vol);
			void setPan(SoundResource& source, float vol);
			void setPan(AudioMixerSource& source, float vol);
			void setFadePan(SoundResource& source, float val, std::chrono::duration<double> time);
			void setFade(SoundResource& source, float val, std::chrono::duration<double> time);
			void setLoop(SoundResource& source, bool isLooped);
		};
	}
};

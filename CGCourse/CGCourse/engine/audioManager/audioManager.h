
#pragma once

#include <filesystem>
#include <string>
#include <irrKlang.h>
import glmath;

namespace KUMA {
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
			irrklang::ISoundEngine* soundEngine;
		public:
			AudioManager() {
				soundEngine = irrklang::createIrrKlangDevice();
			}

			~AudioManager() {
				soundEngine->drop();
			}

			void play2d(Sound2d sound, bool isLooped) {
				auto soundTracker = soundEngine->play2D(sound.path.c_str(), isLooped);
			}

			void play3d(Sound3d sound, bool isLooped) {
				auto soundTracker = soundEngine->play3D(sound.path.c_str(), irrklang::vec3df(sound.pos.x, sound.pos.y, sound.pos.z), isLooped);
			}

			void setListenerPos(MATHGL::Vector3 pos, MATHGL::Vector3 dir) {
				soundEngine->setListenerPosition(irrklang::vec3df(pos.x, pos.y, pos.z), irrklang::vec3df(dir.x, dir.y, dir.z));
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
	}

};

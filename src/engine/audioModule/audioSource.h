#pragma once
#include <memory>
#include <optional>
#include <string>
#include "soloud.h"
#include <utilsModule/reflection/reflection.h>
#include <serdepp/attribute/make_optional.hpp>

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

		static auto GetMembers() {
			return std::tuple{
				IKIGAI::UTILS::MakeMemberInfo("type", &IKIGAI::AUDIO::SoundConfig::type),
				IKIGAI::UTILS::MakeMemberInfo("volume", &IKIGAI::AUDIO::SoundConfig::volume),
				IKIGAI::UTILS::MakeMemberInfo("state", &IKIGAI::AUDIO::SoundConfig::state),
				IKIGAI::UTILS::MakeMemberInfo("pan", &IKIGAI::AUDIO::SoundConfig::pan),
				IKIGAI::UTILS::MakeMemberInfo("is3D", &IKIGAI::AUDIO::SoundConfig::is3D),
				IKIGAI::UTILS::MakeMemberInfo("isLooped", &IKIGAI::AUDIO::SoundConfig::isLooped),
				IKIGAI::UTILS::MakeMemberInfo("pathSoundSource", &IKIGAI::AUDIO::SoundConfig::pathSoundSource)
			};
		}

		template<class Context>
		constexpr static auto serde(Context& context, SoundConfig& value) {
			using Self = SoundConfig;
			using namespace serde::attribute;
			serde::serde_struct(context, value)
				.field(&Self::type, "Type")
				.field(&Self::volume, "Volume")
				.field(&Self::state, "State")
				.field(&Self::pan, "Pan")
				.field(&Self::is3D, "Is3D")
				.field(&Self::isLooped, "IsLooped")
				.field(&Self::pathSoundSource, "PathSoundSource");
		}
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

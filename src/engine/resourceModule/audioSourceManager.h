#pragma once
#include <future>
#include <memory>
#include <string>
#include "resourceManager.h"

namespace IKIGAI
{
	namespace AUDIO
	{
		class SoundResource;
	}
}

namespace IKIGAI {
	namespace RESOURCES {
		class AudioSourceLoader : public ResourceManager<AUDIO::SoundResource> {
		public:
			static ResourcePtr<AUDIO::SoundResource> CreateFromFile(const std::string& path);
		private:
			virtual ResourcePtr<AUDIO::SoundResource> createResource(const std::string& path) override;
			virtual ResourcePtr<AUDIO::SoundResource> createResource(const std::string& path, ELoadingType type) override {
				return createResource(path, type, std::any());
			}
			virtual ResourcePtr<AUDIO::SoundResource> createResource(const std::string& path, ELoadingType type, std::any data) override;
		};
	}
}

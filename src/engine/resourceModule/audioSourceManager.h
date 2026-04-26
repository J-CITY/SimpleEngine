#pragma once
#include <memory>
#include <string>
#include "resourceManager.h"
#include "utilsModule/event.h"
#include "utilsModule/idGenerator.h"

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
			// Load without cache and file watching
			static ResourcePtr<AUDIO::SoundResource> CreateFromFile(const std::string& path);

		private:
			ResourcePtr<AUDIO::SoundResource> createResource(const std::string& path) override;
			ResourcePtr<AUDIO::SoundResource> createResource(const std::string& path, ELoadingType type) override;
			ResourcePtr<AUDIO::SoundResource> createResource(const std::string& path, ELoadingType type, std::any data) override;

			inline static std::unordered_map<std::string, std::vector<IdGenerator<EVENT::Event<>>::id>> fwSubscribersIds;
		};
	}
}

#include "audioSourceManager.h"
#include "ServiceManager.h"
#include <taskModule/taskSystem.h>
#include <audioModule/audioManager.h>
#include <audioModule/audioSource.h>
#include "utilsModule/jsonLoader.h"
#include "utilsModule/pathGetter.h"
#include <resourceModule/fileSystem/fileSystem.h>

namespace IKIGAI::RESOURCES {
	// TODO: add load from resource file and file
	// TODO: add load from ready resource config
	// TODO: add static Reload
	ResourcePtr<AUDIO::SoundResource> AudioSourceLoader::CreateFromFile(const std::string& path) {
		auto content = ServiceManager::Get<FileSystem>().getFile(path)->readStr();
		auto configRes = UTILS::FromJsonStr<AUDIO::SoundConfig>(content);
		if (configRes.isErr()) {
			// TODO: add assert and return dummy resource
		}
		auto config = configRes.unwrap();
		ResourcePtr<AUDIO::SoundResource> resource = ResourcePtr<AUDIO::SoundResource>(new AUDIO::SoundResource(path, config), [](AUDIO::SoundResource* m) {
			AUDIO::AudioManagerAL::GetInstance().removeSource(m->id);
			ServiceManager::Get<AudioSourceLoader>().unloadResource(m->path);
		});

		AUDIO::AudioManagerAL::GetInstance().addSource(*resource);
		resource->path = path;
		return resource;
	}

	ResourcePtr<AUDIO::SoundResource> AudioSourceLoader::createResource(const std::string& path) {
		return CreateFromFile(path);
	}

	ResourcePtr<AUDIO::SoundResource> AudioSourceLoader::createResource(const std::string& path, ELoadingType type) {
		return createResource(path, type, std::any());
	}

	ResourcePtr<AUDIO::SoundResource> AudioSourceLoader::createResource(const std::string& path, ELoadingType type, std::any data) {
		if (type == ELoadingType::RESOURCE) {
			return CreateFromFile(path);
		}
		return createResource(path);
	}
}

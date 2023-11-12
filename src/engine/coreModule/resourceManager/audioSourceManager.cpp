#include "audioSourceManager.h"
#include "ServiceManager.h"
#include <taskModule/taskSystem.h>
#include <utilsModule/jsonParser/jsonParser.h>
#include <audioModule/audioManager.h>
#include <audioModule/audioSource.h>


using namespace IKIGAI;
using namespace IKIGAI::RESOURCES;


ResourcePtr<AUDIO::SoundResource> AudioSourceLoader::createResource(const std::string& path) {
	return CreateFromFile(path);
}

ResourcePtr<AUDIO::SoundResource> AudioSourceLoader::CreateFromFile(const std::string& path) {
	std::string realPath = UTILS::getRealPath(path);

	auto configRes = UTILS::FromJson<AUDIO::SoundConfig>(path);

	if (configRes.isErr()) {
		//problem
	}
	auto config = configRes.unwrap();

	ResourcePtr<AUDIO::SoundResource> resource = ResourcePtr<AUDIO::SoundResource>(new AUDIO::SoundResource(path, config), [](AUDIO::SoundResource* m) {
		AUDIO::AudioManagerAL::GetInstance().removeSource(m->id);
		ServiceManager::Get<AudioSourceLoader>().unloadResource<TextureLoader>(m->path);
	});

	AUDIO::AudioManagerAL::GetInstance().addSource(*resource);
	resource->path = path;
	return resource;
}

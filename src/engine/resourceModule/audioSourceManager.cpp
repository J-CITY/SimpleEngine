#include "audioSourceManager.h"
#include "ServiceManager.h"
#include <taskModule/taskSystem.h>
#include <audioModule/audioManager.h>
#include <audioModule/audioSource.h>
#include "utilsModule/jsonLoader.h"
#include "utilsModule/pathGetter.h"
#include <resourceModule/fileSystem/fileSystem.h>
#include "utilsModule/assertion.h"
#include "fileWatcher.h"

namespace IKIGAI::RESOURCES {
	using SoundDeleter = std::function<void(AUDIO::SoundResource*)>;

	template <typename T, typename... Args>
	std::shared_ptr<T> AllocateSoundResource(UTILS::IAllocator* allocator, SoundDeleter customDeleter, Args&&... args) {
		void* raw = allocator ? allocator->allocate(sizeof(T)) : ::operator new(sizeof(T));
		auto* obj = new(raw) T(std::forward<Args>(args)...);

		SoundDeleter finalDeleter = [allocator, customDeleter](AUDIO::SoundResource* p) {
			if (customDeleter) {
				customDeleter(p);
			}
			p->~SoundResource();
			if (allocator) allocator->deallocate(p);
			else ::operator delete(p);
			};

		return std::shared_ptr<T>(obj, std::move(finalDeleter));
	}

	ResourcePtr<AUDIO::SoundResource> AudioSourceLoader::CreateFromFile(const std::string& path, UTILS::IAllocator* allocator, AudioDeleter deleter) {
		AUDIO::SoundConfig config;
		config.pathSoundSource = path;
		config.type = AUDIO::SoundType::SOUND;
		config.volume = -1.0f;
		config.state = AUDIO::SoundStatus::STOP;
		config.pan = 0.0f;
		config.is3D = false;
		config.isLooped = false;

		auto resource = AllocateSoundResource<AUDIO::SoundResource>(allocator, std::move(deleter), path, config);
		AUDIO::AudioManagerAL::GetInstance().addSource(*resource);
		resource->path = path;

		AddFileWatchSubscribe(path, path, resource);
		return resource;
	}

	ResourcePtr<AUDIO::SoundResource> AudioSourceLoader::CreateFromResource(const std::string& path, UTILS::IAllocator* allocator, AudioDeleter deleter) {
		AUDIO::SoundConfig config;
		if (auto it = sResourceCache.find(path); it != sResourceCache.end()) {
			config = it->second;
		} else {
			auto content = ServiceManager::Get<FileSystem>().getFile(path)->readStr();
			auto configRes = UTILS::FromJsonStr<AUDIO::SoundConfig>(content);
			if (configRes.isErr()) {
				ASSERT(std::string("Can`t load sound config: " + path).c_str());
				return nullptr;
			}
			config = configRes.unwrap();
			sResourceCache[path] = config;
		}

		auto resource = AllocateSoundResource<AUDIO::SoundResource>(allocator, std::move(deleter), path, config);

		AUDIO::AudioManagerAL::GetInstance().addSource(*resource);
		resource->path = path;

		AddFileWatchSubscribe(path, config.pathSoundSource, resource);
		return resource;
	}

	ResourcePtr<AUDIO::SoundResource> AudioSourceLoader::createResource(const std::string& path) {
		if (path.ends_with(".sound")) {
			return CreateFromResource(path);
		}
		return CreateFromFile(path);
	}

	ResourcePtr<AUDIO::SoundResource> AudioSourceLoader::createResource(const std::string& path, ELoadingType type) {
		return createResource(path, type, std::any());
	}

	ResourcePtr<AUDIO::SoundResource> AudioSourceLoader::createResource(const std::string& path, ELoadingType type, std::any data) {
		if (type == ELoadingType::RESOURCE) {
			return CreateFromResource(path);
		}
		if (type == ELoadingType::FILE) {
			return CreateFromFile(path);
		}
		return createResource(path);
	}

	void AudioSourceLoader::UpdateFileWatchResource(const std::string& configPath, const std::string& soundPath, std::weak_ptr<AUDIO::SoundResource> weakSound) {
		if (auto sound = weakSound.lock()) {
			if (!configPath.empty() && configPath != soundPath) {
				auto content = ServiceManager::Get<FileSystem>().getFile(configPath)->readStr();
				auto configRes = UTILS::FromJsonStr<AUDIO::SoundConfig>(content);
				if (configRes.isOk()) {
					auto config = configRes.unwrap();
					sResourceCache[configPath] = config;
					sound->data = config;
				}
			}
			AddFileWatchSubscribe(configPath, soundPath, weakSound);
			// Simply reloading via removing and adding source back to trigger reload in audioManagerAL if needed
			AUDIO::AudioManagerAL::GetInstance().removeSource(*sound);
			AUDIO::AudioManagerAL::GetInstance().addSource(*sound);
		}
	}

	void AudioSourceLoader::AddFileWatchSubscribe(const std::string& configPath, const std::string& soundPath, std::weak_ptr<AUDIO::SoundResource> weakSound) {
		auto fwCb = [configPath, soundPath, weakSound](RESOURCES::FileWatcher::FileStatus status) {
			switch (status) {
			case RESOURCES::FileWatcher::FileStatus::MODIFIED: {
				UnsubscribeFileWatch(configPath);
				UpdateFileWatchResource(configPath, soundPath, weakSound);
				break;
			}
			case RESOURCES::FileWatcher::FileStatus::DEL:
			case RESOURCES::FileWatcher::FileStatus::CREATE:
				break;
			}
		};

		auto saveCb = [configPath](auto e) {
			AudioSourceLoader::fwSubscribersIds[configPath].push_back(e);
		};
		if (!configPath.empty() && configPath != soundPath) {
			RESOURCES::FileWatcher::getInstance()->addDeferred(configPath, fwCb, saveCb);
		}
		
		auto soundSaveCb = [soundPath](auto e) {
			AudioSourceLoader::fwSubscribersIds[soundPath].push_back(e);
		};
		RESOURCES::FileWatcher::getInstance()->addDeferred(soundPath, fwCb, soundSaveCb);
	}

	void AudioSourceLoader::UnsubscribeFileWatch(const std::string& path) {
		if (fwSubscribersIds.contains(path)) {
			for (auto& e : fwSubscribersIds[path]) {
				RESOURCES::FileWatcher::getInstance()->removeDeferred(path, e);
			}
			fwSubscribersIds.erase(path);
		}
	}
}

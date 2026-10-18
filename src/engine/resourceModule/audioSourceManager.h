#pragma once
#include <memory>
#include <string>
#include "resourceManager.h"
#include "utilsModule/event.h"
#include "utilsModule/idGenerator.h"
#include "audioModule/audioSource.h"
#include "utilsModule/memory/memory.h"
#include <functional>

namespace IKIGAI::RESOURCES {
	using AudioDeleter = std::function<void(AUDIO::SoundResource*)>;

	class AudioSourceLoader : public ResourceManager<AUDIO::SoundResource, AUDIO::SoundConfig> {
	public:
		// Load without cache and file watching
		static ResourcePtr<AUDIO::SoundResource> CreateFromFile(const std::string& path, UTILS::IAllocator* allocator = nullptr, AudioDeleter deleter = nullptr);
		static ResourcePtr<AUDIO::SoundResource> CreateFromResource(const std::string& path, UTILS::IAllocator* allocator = nullptr, AudioDeleter deleter = nullptr);
		static ResourcePtr<AUDIO::SoundResource> CreateFromResource(const AUDIO::SoundConfig& config, UTILS::IAllocator* allocator = nullptr, AudioDeleter deleter = nullptr);

	private:
        COROUTINE::Task<ResourcePtr<AUDIO::SoundResource>> createResourceTask(
            std::string path, ELoadingType type, std::shared_ptr<ResourceLoadControl> control) override;
		ResourcePtr<AUDIO::SoundResource> createResource(const std::string& path) override;
		ResourcePtr<AUDIO::SoundResource> createResource(const std::string& path, ELoadingType type) override;
		ResourcePtr<AUDIO::SoundResource> createResource(const std::string& path, ELoadingType type, std::any data) override;

		virtual bool reloadResource(std::weak_ptr<AUDIO::SoundResource> weakRes, const std::string& config) override;
	};
}

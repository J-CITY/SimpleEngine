#pragma once
#include <memory>
#include <string>
#include "resourceManager.h"
#include "utilsModule/event.h"
#include "utilsModule/idGenerator.h"
#include "audioModule/audioSource.h"
#include "utilsModule/memoryAlloc.h"
#include <functional>

namespace IKIGAI::RESOURCES {
	using AudioDeleter = std::function<void(AUDIO::SoundResource*)>;

	class AudioSourceLoader : public ResourceManager<AUDIO::SoundResource> {
	public:
		// Load without cache and file watching
		static ResourcePtr<AUDIO::SoundResource> CreateFromFile(const std::string& path, UTILS::IAllocator* allocator = nullptr, AudioDeleter deleter = nullptr);
		static ResourcePtr<AUDIO::SoundResource> CreateFromResource(const std::string& path, UTILS::IAllocator* allocator = nullptr, AudioDeleter deleter = nullptr);

	private:
		ResourcePtr<AUDIO::SoundResource> createResource(const std::string& path) override;
		ResourcePtr<AUDIO::SoundResource> createResource(const std::string& path, ELoadingType type) override;
		ResourcePtr<AUDIO::SoundResource> createResource(const std::string& path, ELoadingType type, std::any data) override;

		static void UpdateFileWatchResource(const std::string& configPath, const std::string& soundPath, std::weak_ptr<AUDIO::SoundResource> weakSound);
		static void AddFileWatchSubscribe(const std::string& configPath, const std::string& soundPath, std::weak_ptr<AUDIO::SoundResource> weakSound);
		static void UnsubscribeFileWatch(const std::string& path);

		inline static std::unordered_map<std::string, AUDIO::SoundConfig> sResourceCache;
		inline static std::unordered_map<std::string, std::vector<IdGenerator<EVENT::Event<>>::ID>> fwSubscribersIds;
	};
}

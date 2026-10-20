#pragma once

#include "resourceRequest.h"
#include <any>
#include <future>
#include <mutex>
#include <unordered_set>

#include "fileSystem/fileSystem.h"
#include "fileWatcher.h"
#include "serviceManager.h"
#include "utilsModule/assertion.h"
#include "utilsModule/event.h"
#include "utilsModule/idGenerator.h"
#include "utilsModule/jsonLoader.h"
#include "utilsModule/stringUtils.h"

namespace IKIGAI {
namespace RESOURCES {
	template <typename T> using ResourcePtr = std::shared_ptr<T>;

	enum class ELoadingType {
		RESOURCE, // JSON descriptor based
		FILE, // Raw file based
		MEMORY // Memory/Generated
	};

	enum class ELoadingPolicy { Synchronous, Asynchronous, OnDemand };

	class IResourceManager {
	public:
		virtual ~IResourceManager() = default;
		virtual void unloadResource(const std::string& path) = 0;
	};

	template <class ResourceT, class ConfigT> class ResourceManager : public IResourceManager {
	public:
		using ResourceDeleter = std::function<void(ResourceT*)>;

		virtual ~ResourceManager() override {
			mRequests.shutdown();
			mLifetime.reset();
		}

		static std::string NormalizePath(std::string path) {
			path = UTILS::ReplaceSubstrings(path, "\\", "/");
			for (const auto prefix : { "assets/engine/", "assets/game/" }) {
				if (const auto pos = path.find(prefix); pos != std::string::npos) {
					path = path.substr(pos + std::char_traits<char>::length(prefix));
				}
			}
			return "/" + std::filesystem::path(path).relative_path().lexically_normal().generic_string();
		}
		ResourceRequest<ResourceT> requestResource(std::string path, ELoadingType type = ELoadingType::RESOURCE) {
			path = NormalizePath(std::move(path));
			auto& scheduler = ServiceManager::Get<COROUTINE::Scheduler>();
			auto& files = ServiceManager::Get<FileSystem>();
			const auto generation = files.generation();
			auto key = std::to_string(generation) + ":" + std::to_string(static_cast<int>(type)) + ":" + path;
			return mRequests.request(scheduler, std::move(key), [this, path, type, generation, &files](auto control) {
				control->valid = [&files, generation] { return files.generation() == generation; };
				return createResourceTask(path, type, std::move(control));
			});
		}
		COROUTINE::Task<ResourcePtr<ResourceT>> loadResourceTask(std::string path, ELoadingType type = ELoadingType::RESOURCE) {
			auto request = requestResource(std::move(path), type);
			co_return co_await request.wait();
		}

		ResourcePtr<ResourceT> loadResource(const std::string& path) {
			return loadResource(path, ELoadingType::RESOURCE);
		}

		ResourcePtr<ResourceT> loadResource(const std::string& path, ELoadingType type) {
			return loadResource(path, type, std::any());
		}

		ResourcePtr<ResourceT> loadResource(const std::string& path, ELoadingType type, std::any data) {
			const auto _path = NormalizePath(path);

			{
				std::lock_guard<std::recursive_mutex> lock(mMutex);
				if (auto resource = getResource(_path)) {
					return resource;
				}
			}

			static thread_local std::unordered_set<std::string> creating;
			const auto creationKey = std::to_string(static_cast<int>(type)) + ":" + _path;
			if (!creating.insert(creationKey).second) {
				throw std::runtime_error("Resource dependency cycle: " + _path);
			}
			struct CreationGuard {
				std::unordered_set<std::string>& set;
				std::string key;
				~CreationGuard() {
					set.erase(key);
				}
			} guard { creating, creationKey };
			auto newResource = createResource(_path, type, data);
			if (newResource) {
				return registerResource(_path, newResource);
			}
			return nullptr;
		}

		[[deprecated("Use requestResource or loadResourceTask on the main thread")]]
		std::future<ResourcePtr<ResourceT>> loadResourceAsync(const std::string& path, ELoadingType type) {
			// TODO: use task system or corutines
			return std::async(std::launch::async, [this, path, type]() { return this->loadResource(path, type); });
		}

		[[deprecated("Use requestResource or loadResourceTask on the main thread")]]
		std::future<ResourcePtr<ResourceT>> loadResourceAsync(const std::string& path, ELoadingType type, std::any data) {
			// TODO: use task system or corutines
			return std::async(std::launch::async, [this, path, type, data]() { return this->loadResource(path, type, data); });
		}

		void unloadResource(const std::string& path) override {
			std::lock_guard<std::recursive_mutex> lock(mMutex);
			resources.erase(NormalizePath(path));
		}

		std::unordered_map<std::string, std::weak_ptr<ResourceT>>& getResources() {
			return resources;
		}

		virtual ResourcePtr<ResourceT> createResource(const std::string& p_path) = 0;

		virtual ResourcePtr<ResourceT> createResource(const std::string& p_path, ELoadingType type) {
			return createResource(p_path, type, std::any());
		}

		virtual ResourcePtr<ResourceT> createResource(const std::string& p_path, ELoadingType type, std::any data) {
			return createResource(p_path);
		}

		static void SetAssetPaths(const std::string& projectAssetsPath, const std::string& engineAssetsPath) {
			PROJECT_ASSETS_PATH = projectAssetsPath;
			ENGINE_ASSETS_PATH = engineAssetsPath;
		}

	protected:
		virtual COROUTINE::Task<ResourcePtr<ResourceT>> createResourceTask(
			std::string path, ELoadingType type, std::shared_ptr<ResourceLoadControl> control) {
			control->check();
			co_await COROUTINE::YieldIfBudgetExceeded();
			control->check();
			co_return loadResource(path, type);
		}
		ResourceRequests<ResourceT> mRequests;
		ResourcePtr<ResourceT> getResource(const std::string& path) {
			// Assumes lock is held by caller if called internally,
			// but let's use recursive_mutex so we can lock again just in case
			std::lock_guard<std::recursive_mutex> lock(mMutex);
			const auto generation = ServiceManager::Get<FileSystem>().generation();
			if (mGeneration != generation) {
				resources.clear();
				sResourceCache.clear();
				mGeneration = generation;
			}
			if (auto resource = resources.find(NormalizePath(path)); resource != resources.end()) {
				return resource->second.lock();
			}
			return nullptr;
		}

		ResourcePtr<ResourceT> registerResource(const std::string& path, std::shared_ptr<ResourceT> res) {
			std::lock_guard<std::recursive_mutex> lock(mMutex);
			resources[NormalizePath(path)] = res;
			return res;
		}

		// config cache
		static ConfigT LoadConfig(const std::string& path) {
			auto file = ServiceManager::Get<FileSystem>().getFile(path, FileMode::READ);
			if (!file || !file->isValid()) {
				ASSERT("TextureLoader: cannot open .res file");
			}
			auto parseResult = UTILS::FromJsonStr<ConfigT>(file->readStr());
			if (parseResult.isErr()) {
				ASSERT("TextureLoader: failed to parse .res file");
			}
			auto res = parseResult.unwrap();
			res.path = path;
			return res;
		}

		static bool HasConfig(const std::string& path) {
			return sResourceCache.contains(path);
		}

		static std::optional<ConfigT> GetConfig(const std::string& path) {
			if (auto it = sResourceCache.find(path); it != sResourceCache.end()) {
				return it->second;
			}
			return std::nullopt;
		}

		static void AddConfigToCache(const std::string& path, const ConfigT& config) {
			sResourceCache[path] = config;
		}

		static void RemoveConfigFromCache(const std::string& path) {
			sResourceCache.erase(path);
		}

		virtual void addFileWatchSubscribe(
			const std::string& mainPath, const std::unordered_set<std::string>& watchPaths, std::weak_ptr<ResourceT> weakRes) {
#ifndef USE_FILE_WATCHER
			(void)mainPath;
			(void)watchPaths;
			(void)weakRes;
			return;
#endif
			auto fwCb = [this, mainPath, watchPaths, weakRes, lifetime = std::weak_ptr<int>(mLifetime)](RESOURCES::FileWatcher::FileStatus status) {
				if (lifetime.expired()) {
					return;
				}
				if (status == RESOURCES::FileWatcher::FileStatus::MODIFIED) {
					if (weakRes.lock()) {
						// A rejected edit keeps the existing subscriptions and their observed
						// timestamps. Re-registering here could swallow the next valid save.
						if (reloadResource(weakRes, mainPath)) {
							unsubscribeFileWatch(mainPath);
							addFileWatchSubscribe(mainPath, watchPaths, weakRes);
						}
					}
				}
			};

			for (const auto& path : watchPaths) {
				auto saveCb = [mainPath, path](auto e) { sFWSubscribersIds[mainPath].emplace_back(path, e); };
				RESOURCES::FileWatcher::getInstance()->addDeferred(path, fwCb, saveCb);
			}
		}

		virtual void unsubscribeFileWatch(const std::string& mainPath) {
#ifndef USE_FILE_WATCHER
			(void)mainPath;
			return;
#endif
			for (auto& e : sFWSubscribersIds[mainPath]) {
				RESOURCES::FileWatcher::getInstance()->removeDeferred(e.first, e.second);
			}
			sFWSubscribersIds.erase(mainPath);
		}

		virtual bool reloadResource(std::weak_ptr<ResourceT> weakRes, const std::string& config) = 0;

		virtual ResourceDeleter createCacheDeleter(const std::string& path) {
			return [this, path, lifetime = std::weak_ptr<int>(mLifetime)](ResourceT* ptr) {
				// Render targets/materials may outlive the resource loader during shutdown.
				if (!lifetime.expired()) {
					// An older generation may die after a replacement was
					// registered under the same key. Keep the live replacement.
					std::lock_guard<std::recursive_mutex> lock(mMutex);
					auto it = resources.find(NormalizePath(path));
					if (it != resources.end() && it->second.expired()) {
						resources.erase(it);
					}
				}
			};
		}

		mutable std::recursive_mutex mMutex;

		// Кэш разобранных дескрипторов
		inline static std::unordered_map<std::string, ConfigT> sResourceCache;
		// FW ids
		inline static std::unordered_map<std::string, std::vector<std::pair<std::string, IdGenerator<EVENT::Event<>>::ID>>> sFWSubscribersIds;

	private:
		uint64_t mGeneration = ~uint64_t { 0 };
		std::shared_ptr<int> mLifetime = std::make_shared<int>(0);
		inline static std::string PROJECT_ASSETS_PATH = "";
		inline static std::string ENGINE_ASSETS_PATH = "";

		std::unordered_map<std::string, std::weak_ptr<ResourceT>> resources;
	};
};
};

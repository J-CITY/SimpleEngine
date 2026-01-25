#pragma once

#include <mutex>
#include <future>
#include <any>

#include "utilsModule/stringUtils.h"

namespace IKIGAI {
	namespace RESOURCES {
		template<typename T>
		using ResourcePtr = std::shared_ptr<T>;

		enum class ELoadingType {
			RESOURCE,   // JSON descriptor based
			FILE,       // Raw file based
			DESCRIPTOR, // Descriptor struct based
			MEMORY      // Memory/Generated
		};

		enum class ELoadingPolicy {
			Synchronous,
			Asynchronous,
			OnDemand
		};

		class IResourceManager {
		public:
			virtual ~IResourceManager() = default;
			virtual void unloadResource(const std::string& path) = 0;
		};

		template<typename T>
		class ResourceManager : public IResourceManager {
		public:
			virtual ~ResourceManager() = default;

			//TODO: думаю можно сделать ресурс для текстуры с её описанием и грузить его
			//template<typename T>
			ResourcePtr<T> loadResource(const std::string& path) {
				return loadResource(path, ELoadingType::RESOURCE);
			}

			ResourcePtr<T> loadResource(const std::string& path, ELoadingType type) {
				return loadResource(path, type, std::any());
			}

			ResourcePtr<T> loadResource(const std::string& path, ELoadingType type, std::any data) {
				std::string _path = UTILS::ReplaceSubstrings(path, "\\", "/");
				auto pos = _path.find("assets/engine/");
				if (pos != std::string::npos) {
					_path = _path.substr(pos + 14);
				}
				pos = _path.find("assets/game/");
				if (pos != std::string::npos) {
					_path = _path.substr(pos + 12);
				}

				{
					std::lock_guard<std::recursive_mutex> lock(mMutex);
					if (auto resource = getResource(_path)) {
						return resource;
					}
				}

				auto newResource = createResource(_path, type, data);
				if (newResource) {
					return registerResource(_path, newResource);
				}
				else {
					return nullptr;
				}
			}

			std::future<ResourcePtr<T>> loadResourceAsync(const std::string& path, ELoadingType type) {
				//TODO: use task system or corutines
				return std::async(std::launch::async, [this, path, type]() {
					return this->loadResource(path, type);
				});
			}

			std::future<ResourcePtr<T>> loadResourceAsync(const std::string& path, ELoadingType type, std::any data) {
				//TODO: use task system or corutines
				return std::async(std::launch::async, [this, path, type, data]() {
					return this->loadResource(path, type, data);
				});
			}

			//template<typename T>
			void unloadResource(const std::string& path) override {
				std::lock_guard<std::recursive_mutex> lock(mMutex);
				resources.erase(path);
			}

			//template<typename T>
			std::unordered_map<std::string, std::weak_ptr<T>>& getResources() {
				// Warning: returning reference to map is not thread-safe if caller iterates it without lock
				// Ideally we should provide thread-safe iteration or copy
				return resources;
			}

			static void SetAssetPaths(const std::string& projectAssetsPath, const std::string& engineAssetsPath);

			virtual ResourcePtr<T> createResource(const std::string& p_path) = 0;
			
			virtual ResourcePtr<T> createResource(const std::string& p_path, ELoadingType type) {
				return createResource(p_path, type, std::any());
			}

			virtual ResourcePtr<T> createResource(const std::string& p_path, ELoadingType type, std::any data) {
				return createResource(p_path);
			}
		protected:
			//template<typename T>
			ResourcePtr<T> getResource(const std::string& path) {
				// Assumes lock is held by caller if called internally, 
				// but let's use recursive_mutex so we can lock again just in case
				std::lock_guard<std::recursive_mutex> lock(mMutex);
				if (auto resource = resources.find(path); resource != resources.end()) {
					return resource->second.lock();
				}
				return nullptr;
			}

			//template<typename T>
			ResourcePtr<T> registerResource(const std::string& path, std::shared_ptr<T> res) {
				std::lock_guard<std::recursive_mutex> lock(mMutex);
				resources[path] = res;
				return res;
			}
			
			//static std::string getRealPath(const std::string& p_path) {
			//	std::string result;
			//	if (std::filesystem::exists(ENGINE_ASSETS_PATH + p_path)) {
			//		result = Config::ROOT + ENGINE_ASSETS_PATH + p_path;
			//	}
			//	else {
			//		result = Config::ROOT + PROJECT_ASSETS_PATH + p_path;
			//	}
			//	return result;
			//}
		public: // Made public for direct lock access if needed
			mutable std::recursive_mutex mMutex;
		private:
			inline static std::string PROJECT_ASSETS_PATH = "";
			inline static std::string ENGINE_ASSETS_PATH = "";

			std::unordered_map<std::string, std::weak_ptr<T>> resources;
		};

		template<typename T>
		inline void ResourceManager<T>::SetAssetPaths(const std::string& projectAssetsPath, const std::string& engineAssetsPath) {
			PROJECT_ASSETS_PATH = projectAssetsPath;
			ENGINE_ASSETS_PATH = engineAssetsPath;
		}
	};
};

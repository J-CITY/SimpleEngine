#pragma once

#include <filesystem>
#include <memory>
#include <unordered_map>

#include "../config.h"

namespace KUMA {
	namespace RESOURCES {
		template<typename T>
		class ResourceManager {
		public:
			template<typename T>
			std::shared_ptr<T> loadResource(const std::string& path) {
				if (auto resource = getResource(path)) {
					return resource;
				}
				else {
					auto newResource = createResource(path);
					if (newResource) {
						return registerResource(path, newResource);
					}
					else {
						return nullptr;
					}
				}
			}

			template<typename T>
			void unloadResource(const std::string& path) {
				if (auto resource = getResource(path)) {
					destroyResource(resource);
					resources.erase(path);
				}
			}

			template<typename T>
			std::unordered_map<std::string, std::weak_ptr<T>>& getResources() {
				return resources;
			}

			static void SetAssetPaths(const std::string& projectAssetsPath, const std::string& engineAssetsPath);

			virtual std::shared_ptr<T> createResource(const std::string& p_path) = 0;
			virtual void destroyResource(std::shared_ptr<T> res) = 0;
		protected:
			template<typename T>
			std::shared_ptr<T> getResource(const std::string& path) {
				if (auto resource = resources.find(path); resource != resources.end()) {
					return resource->second.lock();
				}
				return nullptr;
			}

			template<typename T>
			std::shared_ptr<T> registerResource(const std::string& path, std::shared_ptr<T> res) {
				if (auto resource = getResource<T>(path)) {
					destroyResource(resource);
				}
				resources[path] = res;
				return res;
			}
			
			static std::string getRealPath(const std::string& p_path) {
				std::string result;
				if (std::filesystem::exists(ENGINE_ASSETS_PATH + p_path)) {
					result = Config::ROOT + ENGINE_ASSETS_PATH + p_path;
				}
				else {
					result = Config::ROOT + PROJECT_ASSETS_PATH + p_path;
				}
				return result;
			}
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

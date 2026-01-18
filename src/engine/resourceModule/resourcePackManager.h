#pragma once
#include <string>
#include <map>
#include <vector>
#include <functional>
#include "resourceManager.h"
#include <nlohmann/json.hpp>

#include <any>

namespace IKIGAI::RESOURCES {
	class ResourcePackManager {
	public:
		using LoaderFunc = std::function<std::any(const std::string& path, ELoadingType type, ELoadingPolicy policy)>;
		using UnloaderFunc = std::function<void(const std::string& path)>;

		static ResourcePackManager& Instance() {
			static ResourcePackManager instance;
			return instance;
		}

		void registerResourceType(const std::string& resourceType, LoaderFunc loader, UnloaderFunc unloader) {
			mLoaders[resourceType] = loader;
			mUnloaders[resourceType] = unloader;
		}

		void loadPack(const std::string& path, ELoadingPolicy policy = ELoadingPolicy::Synchronous);
		void unloadPack(const std::string& path);

	private:
		std::map<std::string, LoaderFunc> mLoaders;
		std::map<std::string, UnloaderFunc> mUnloaders;

		struct LoadedResource {
			std::string type;
			std::string path;
		};
		// Map PackPath -> List of loaded resources
		std::map<std::string, std::vector<LoadedResource>> mLoadedPacks;
		
		std::vector<std::any> mAsyncFutures;
	};
}

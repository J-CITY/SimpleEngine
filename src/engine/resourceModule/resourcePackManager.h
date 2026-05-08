#pragma once
#include <string>
#include <map>
#include <vector>
#include <functional>
#include <algorithm>
#include "resourceManager.h"
#include <nlohmann/json.hpp>

#include <any>

namespace IKIGAI::RESOURCES {
	class ResourcePackManager {
	public:
		using LoaderFunc   = std::function<std::any(const std::string& path, ELoadingType type, ELoadingPolicy policy)>;
		using UnloaderFunc = std::function<void(const std::string& path)>;

		struct ResourceLoaderEntry {
			LoaderFunc   loader;
			UnloaderFunc unloader;
			/// Lower value = loaded first.
			/// Recommended defaults:
			///   skeleton=0, animation=10, blendspace=20,
			///   model/texture/shader/material/audio=50
			int priority = 50;
		};

		static ResourcePackManager& Instance() {
			static ResourcePackManager instance;
			return instance;
		}

		void registerResourceType(
			const std::string& resourceType,
			LoaderFunc loader,
			UnloaderFunc unloader,
			int priority = 50) {
			mLoaders[resourceType] = ResourceLoaderEntry{std::move(loader), std::move(unloader), priority};
		}

		void loadPack(const std::string& path, ELoadingPolicy policy = ELoadingPolicy::Synchronous);
		void unloadPack(const std::string& path);

	private:
		std::map<std::string, ResourceLoaderEntry> mLoaders;
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

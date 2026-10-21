#pragma once

#include "fileSystem/fileSystem.h"
#include "serviceManager.h"
#include "utilsModule/assertion.h"
#include "utilsModule/jsonLoader.h"
#include <optional>
#include <string>
#include <unordered_map>

namespace IKIGAI::RESOURCES {
// Keep descriptor caches isolated by resource type, including for shared descriptor types.
template <class ResourceT, class ConfigT> class ResourceConfigCache {
protected:
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

	static void ClearConfigCache() {
		sResourceCache.clear();
	}
	inline static std::unordered_map<std::string, ConfigT> sResourceCache;
};

// Compiling loaders can own their schema without a second serialized descriptor.
template <class ResourceT> class ResourceConfigCache<ResourceT, void> {
protected:
	static void ClearConfigCache() { }
};
}

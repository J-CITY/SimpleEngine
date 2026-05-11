#include "skeletonStateGraphManager.h"
#include "serviceManager.h"
#include "fileWatcher.h"
#include "utilsModule/jsonLoader.h"
#include "utilsModule/log/loggerDefine.h"

namespace IKIGAI::RESOURCES {
	//TODO: add allocator and custom deleter support
	//TODO: universal work with filewatch
	//TODO: fix work with sResourceCache

	ResourcePtr<SKELETON::SkeletalStateGraph> SkeletonStateGraphLoader::CreateFromResource(const std::string& path, UTILS::IAllocator* allocator, ResourceDeleter deleter) {
		auto config = LoadConfig(path);
		return CreateFromResource(config, allocator, deleter);
	}

	ResourcePtr<SKELETON::SkeletalStateGraph> SkeletonStateGraphLoader::CreateFromResource(const RENDER::SkeletonStateGraphResource& config, UTILS::IAllocator* allocator, ResourceDeleter deleter) {
		return  AllocateResource<SKELETON::SkeletalStateGraph>(allocator, deleter, config);
	}

	bool SkeletonStateGraphLoader::reloadResource(std::weak_ptr<SKELETON::SkeletalStateGraph> weakRes, const std::string& path) {
		if (auto res = weakRes.lock()) {
			auto config = LoadConfig(path);
			AddConfigToCache(path, config);
			res->resource = config;
			addFileWatchSubscribe(path, {path}, weakRes);
			return true;
		}
		return false;
	}

	ResourcePtr<SKELETON::SkeletalStateGraph> SkeletonStateGraphLoader::createResource(const std::string& path) {
		return createResource(path, ELoadingType::RESOURCE);
	}

	ResourcePtr<SKELETON::SkeletalStateGraph> SkeletonStateGraphLoader::createResource(const std::string& path, ELoadingType type) {
		return createResource(path, type, std::any());
	}

	ResourcePtr<SKELETON::SkeletalStateGraph> SkeletonStateGraphLoader::createResource(const std::string& path, ELoadingType type, std::any data) {
		if (type == ELoadingType::RESOURCE) {
			RENDER::SkeletonStateGraphResource config;
			if (HasConfig(path)) {
				config = *GetConfig(path);
			} else {
				config = LoadConfig(path);
				AddConfigToCache(path, config);
			}
			auto graph = CreateFromResource(config);
			addFileWatchSubscribe(path, {path}, graph);
			return graph;
		}
		return nullptr;
	}
}

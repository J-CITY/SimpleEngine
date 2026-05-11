#include "skeletonManager.h"
#include "serviceManager.h"
#include <resourceModule/fileSystem/fileSystem.h>
#include "skeletalModule/skeleton.h"
#include "utilsModule/assertion.h"
#include "utilsModule/jsonLoader.h"
#include "fileWatcher.h"

namespace IKIGAI::RESOURCES {

AssimpParser SkeletonLoader::_ASSIMP;

ResourcePtr<SKELETON::Skeleton> SkeletonLoader::CreateFromFile(const std::string& path, UTILS::IAllocator* allocator, ResourceDeleter deleter) {
	auto skeleton = UTILS::AllocateResource<SKELETON::Skeleton>(allocator, deleter);
	auto data = ServiceManager::Get<FileSystem>().getFile(path)->read();
	if (data.empty()) {
		ASSERT(std::string("SkeletonLoader::CreateFromFile: empty file: " + path).c_str());
		return nullptr;
	}
	if (!_ASSIMP.LoadSkeleton(path, data, *skeleton)) {
		ASSERT(std::string("SkeletonLoader::CreateFromFile: failed to parse: " + path).c_str());
		return nullptr;
	}
	
	return skeleton;
}

ResourcePtr<SKELETON::Skeleton> SkeletonLoader::CreateFromResource(const std::string& path, UTILS::IAllocator* allocator, ResourceDeleter deleter) {
	RENDER::SkeletonResource res = LoadConfig(path);
	return CreateFromResource(res, allocator, deleter);
}

ResourcePtr<SKELETON::Skeleton> SkeletonLoader::CreateFromResource(const RENDER::SkeletonResource& res, UTILS::IAllocator* allocator, ResourceDeleter deleter) {
	return CreateFromFile(res.pathSkeleton, allocator, deleter);
}

ResourcePtr<SKELETON::Skeleton> SkeletonLoader::createResource(const std::string& path) {
	if (path.ends_with(".skeleton")) {
		return createResource(path, ELoadingType::RESOURCE);
	}
	return createResource(path, ELoadingType::FILE);
}

ResourcePtr<SKELETON::Skeleton> SkeletonLoader::createResource(const std::string& path, ELoadingType type) {
	return createResource(path, type, std::any());
}

ResourcePtr<SKELETON::Skeleton> SkeletonLoader::createResource(const std::string& path, ELoadingType type, std::any /*data*/) {
	ResourcePtr<SKELETON::Skeleton> res;
	if (type == ELoadingType::FILE) {
		res = CreateFromFile(path);
		addFileWatchSubscribe(path, {path}, res);
	}
	else if (type == ELoadingType::RESOURCE) {
		RENDER::SkeletonResource config;
		if (HasConfig(path)) {
			config = *GetConfig(path);
		} else {
			config = LoadConfig(path);
			AddConfigToCache(path, config);
		}
		res = CreateFromResource(config);
		addFileWatchSubscribe(path, {path, config.pathSkeleton}, res);
	}
	return res;
}

bool SkeletonLoader::reloadResource(std::weak_ptr<SKELETON::Skeleton> weakRes, const std::string& path) {
	if (auto res = weakRes.lock()) {
		std::unordered_set<std::string> paths = {path};
		RENDER::SkeletonResource config;
		if (path.ends_with(".skeleton")) {
			config = LoadConfig(path);
			AddConfigToCache(path, config);
			paths.insert(config.pathSkeleton);
		}
		else {
			config.path = path;
			config.pathSkeleton = path;
		}

		auto newSkeleton = CreateFromFile(config.pathSkeleton);
		if (newSkeleton) {
			res->setNumJoints(newSkeleton->getNumJolts());
			res->joints() = newSkeleton->joints();
		}

		addFileWatchSubscribe(path, paths, res);
		return true;
	}
	return false;
}

} // namespace IKIGAI::RESOURCES

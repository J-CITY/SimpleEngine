#include "skeletonAnimationManager.h"
#include "serviceManager.h"
#include <resourceModule/fileSystem/fileSystem.h>
#include "skeletalModule/skeleton.h"
#include "skeletalModule/animation.h"
#include "skeletonManager.h"
#include "utilsModule/assertion.h"
#include "utilsModule/jsonLoader.h"
#include "fileWatcher.h"

namespace IKIGAI::RESOURCES {

AssimpParser SkeletonAnimationLoader::_ASSIMP;

ResourcePtr<SKELETON::Animation> SkeletonAnimationLoader::CreateFromFile(
	const std::string& path,
	std::shared_ptr<SKELETON::Skeleton> skeleton,
	bool additive,
	SKELETON::Animation* additiveReference, UTILS::IAllocator* allocator, ResourceDeleter deleter) {

	if (!skeleton) {
		ASSERT("SkeletonAnimationLoader::CreateFromFile: skeleton is null");
		return nullptr;
	}

	auto animation = UTILS::AllocateResource<SKELETON::Animation>(allocator, deleter);
	auto data = ServiceManager::Get<FileSystem>().getFile(path)->read();
	if (data.empty()) {
		ASSERT(std::string("SkeletonAnimationLoader::CreateFromFile: empty file: " + path).c_str());
		return nullptr;
	}
	if (!_ASSIMP.LoadAnimation(path, data, *skeleton, *animation, additive, additiveReference)) {
		ASSERT(std::string("SkeletonAnimationLoader::CreateFromFile: failed to parse: " + path).c_str());
		return nullptr;
	}
	
	return animation;
}

ResourcePtr<SKELETON::Animation> SkeletonAnimationLoader::CreateFromResource(
	const std::string& path,
	std::shared_ptr<SKELETON::Skeleton> skeleton, UTILS::IAllocator* allocator, ResourceDeleter deleter) {

	RENDER::AnimationResource res = LoadConfig(path);
	return CreateFromResource(res, skeleton, allocator, deleter);
}

ResourcePtr<SKELETON::Animation> SkeletonAnimationLoader::CreateFromResource(const RENDER::AnimationResource& res,
	std::shared_ptr<SKELETON::Skeleton> skeleton, UTILS::IAllocator* allocator, ResourceDeleter deleter)
{
	// Resolve additive reference if specified
	SKELETON::Animation* additiveRef = nullptr;
	std::shared_ptr<SKELETON::Animation> additiveRefPtr;
	if (!res.additiveReference.empty()) {
		additiveRefPtr = ServiceManager::Get<SkeletonAnimationLoader>().loadResource(res.additiveReference);
		if (additiveRefPtr) {
			additiveRef = additiveRefPtr.get();
		}
	}

	auto animation = CreateFromFile(res.pathAnimation, skeleton, res.additive, additiveRef);
	return animation;
}

ResourcePtr<SKELETON::Animation> SkeletonAnimationLoader::createResource(const std::string& path) {
	return createResource(path, ELoadingType::RESOURCE, std::any());
}

ResourcePtr<SKELETON::Animation> SkeletonAnimationLoader::createResource(
	const std::string& path, ELoadingType type) {
	return createResource(path, type, std::any());
}

ResourcePtr<SKELETON::Animation> SkeletonAnimationLoader::createResource(const std::string& path, ELoadingType type, std::any data) {
	std::shared_ptr<SKELETON::Skeleton> skeleton;
	bool additive = false;
	std::string additiveRefPath;

	if (data.has_value()) {
		try {
			auto ctx = std::any_cast<AnimationLoadContext>(data);
			skeleton        = ctx.skeleton;
			additive        = ctx.additive;
			additiveRefPath = ctx.additiveReferencePath;
		}
		catch (const std::bad_any_cast&) {
			ASSERT("SkeletonAnimationLoader: invalid std::any context, expected AnimationLoadContext");
			return nullptr;
		}
	}

	ResourcePtr<SKELETON::Animation> res;
	if (type == ELoadingType::FILE) {
		if (!skeleton) {
			ASSERT("SkeletonAnimationLoader: AnimationLoadContext.skeleton is null");
			return nullptr;
		}
		res = CreateFromFile(path, skeleton, additive, nullptr, nullptr, createCacheDeleter(path));
		addFileWatchSubscribe(path, {path}, res);
	}
	else if (type == ELoadingType::RESOURCE) {
		RENDER::AnimationResource config;
		if (HasConfig(path)) {
			config = *GetConfig(path);
		} else {
			config = LoadConfig(path);
			AddConfigToCache(path, config);
		}
		skeleton = ServiceManager::Get<SkeletonLoader>().loadResource(config.skeletonPath);
		if (!skeleton) {
			ASSERT("SkeletonAnimationLoader: AnimationLoadContext.skeleton is null");
			return nullptr;
		}
		res = CreateFromResource(config, skeleton, nullptr, createCacheDeleter(path));
		addFileWatchSubscribe(path, {path, config.pathAnimation}, res);
	}
	return res;
}

bool SkeletonAnimationLoader::reloadResource(std::weak_ptr<SKELETON::Animation> weakRes, const std::string& path)
{
	if (auto res = weakRes.lock()) {
		std::unordered_set<std::string> paths = {path};
		RENDER::AnimationResource config;
		if (path.ends_with(".animation")) {
			config = LoadConfig(path);
			AddConfigToCache(path, config);
			paths.insert(config.pathAnimation);
		} else {
			config.path = path;
			config.pathAnimation = path;
		}

		SKELETON::Animation* additiveRef = nullptr;
		std::shared_ptr<SKELETON::Animation> additiveRefPtr;
		if (!config.additiveReference.empty()) {
			additiveRefPtr = ServiceManager::Get<SkeletonAnimationLoader>().loadResource(config.additiveReference);
			if (additiveRefPtr) {
				additiveRef = additiveRefPtr.get();
			}
		}
		
		auto skeleton = ServiceManager::Get<SkeletonLoader>().loadResource(config.skeletonPath);
		if (!skeleton) {
			ASSERT("SkeletonAnimationLoader: AnimationLoadContext.skeleton is null");
			return false;
		}

		auto newAnim = CreateFromFile(path, nullptr, config.additive, additiveRef);
		if (newAnim) {
			*res = *newAnim;
		}

		addFileWatchSubscribe(path, paths, res);
		return true;
	}
	return false;
}
} // namespace IKIGAI::RESOURCES

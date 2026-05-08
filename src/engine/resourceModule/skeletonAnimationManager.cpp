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
	SKELETON::Animation* additiveReference) {

	if (!skeleton) {
		ASSERT("SkeletonAnimationLoader::CreateFromFile: skeleton is null");
		return nullptr;
	}

	auto animation = std::make_shared<SKELETON::Animation>();
	auto data = ServiceManager::Get<FileSystem>().getFile(path)->read();
	if (data.empty()) {
		ASSERT(std::string("SkeletonAnimationLoader::CreateFromFile: empty file: " + path).c_str());
		return nullptr;
	}
	if (!_ASSIMP.LoadAnimation(path, data, *skeleton, *animation, additive, additiveReference)) {
		ASSERT(std::string("SkeletonAnimationLoader::CreateFromFile: failed to parse: " + path).c_str());
		return nullptr;
	}
	
	// Subscribe to file watch without a config
	AddFileWatchSubscribe(path, path, animation, skeleton);
	
	return animation;
}

ResourcePtr<SKELETON::Animation> SkeletonAnimationLoader::CreateFromResource(
	const std::string& path,
	std::shared_ptr<SKELETON::Skeleton> skeleton) {

	RENDER::AnimationResource res;
	if (auto it = sResourceCache.find(path); it != sResourceCache.end()) {
		res = it->second;
	} else {
		auto content = ServiceManager::Get<FileSystem>().getFile(path)->readStr();
		auto parsed = UTILS::FromJsonStr<RENDER::AnimationResource>(content);
		if (parsed.isErr()) {
			ASSERT(std::string("SkeletonAnimationLoader::CreateFromResource: can't parse: " + path).c_str());
			return nullptr;
		}
		res = parsed.unwrap();
		res.path = path;
		sResourceCache[path] = res;
	}

	// Resolve additive reference if specified
	SKELETON::Animation* additiveRef = nullptr;
	std::shared_ptr<SKELETON::Animation> additiveRefPtr;
	if (!res.additiveReference.empty()) {
		additiveRefPtr = ServiceManager::Get<SkeletonAnimationLoader>().loadResource(res.additiveReference);
		if (additiveRefPtr) {
			additiveRef = additiveRefPtr.get();
		}
	}

	auto animation = CreateFromFile(res.animationPath, skeleton, res.additive, additiveRef);
	if (animation) {
		AddFileWatchSubscribe(path, res.animationPath, animation, skeleton);
	}
	return animation;
}

ResourcePtr<SKELETON::Animation> SkeletonAnimationLoader::createResource(const std::string& path) {
	// Without a skeleton context we can't load an animation
	// Caller must use createResource(path, type, data) with AnimationLoadContext
	ASSERT("SkeletonAnimationLoader::createResource: skeleton context required, use loadResource with AnimationLoadContext");
	return nullptr;
}

ResourcePtr<SKELETON::Animation> SkeletonAnimationLoader::createResource(
	const std::string& path, ELoadingType type) {
	return createResource(path, type, std::any());
}

ResourcePtr<SKELETON::Animation> SkeletonAnimationLoader::createResource(
	const std::string& path, ELoadingType type, std::any data) {

	std::shared_ptr<SKELETON::Skeleton> skeleton;
	bool additive = false;
	std::string additiveRefPath;

	if (data.has_value()) {
		try {
			auto ctx = std::any_cast<AnimationLoadContext>(data);
			skeleton        = ctx.skeleton;
			additive        = ctx.additive;
			additiveRefPath = ctx.additiveReferencePath;
		} catch (const std::bad_any_cast&) {
			ASSERT("SkeletonAnimationLoader: invalid std::any context, expected AnimationLoadContext");
			return nullptr;
		}
	}

	if (!skeleton) {
		ASSERT("SkeletonAnimationLoader: AnimationLoadContext.skeleton is null");
		return nullptr;
	}

	if (type == ELoadingType::FILE) {
		return CreateFromFile(path, skeleton, additive, nullptr);
	}
	if (type == ELoadingType::RESOURCE) {
		return CreateFromResource(path, skeleton);
	}
	// Default: auto-detect by extension
	if (path.ends_with(".animation")) {
		return CreateFromResource(path, skeleton);
	}
	return CreateFromFile(path, skeleton, additive, nullptr);
}

void SkeletonAnimationLoader::Reload(SKELETON::Animation& animation, const std::string& path, std::shared_ptr<SKELETON::Skeleton> skeleton, bool additive, SKELETON::Animation* additiveReference) {
	auto newAnim = CreateFromFile(path, skeleton, additive, additiveReference);
	if (newAnim) {
		animation = *newAnim; // Copy fields (channels, duration, etc.)
	}
}

void SkeletonAnimationLoader::UnsubscribeFileWatch(const std::string& path) {
	if (fwSubscribersIds.contains(path)) {
		for (auto& e : fwSubscribersIds[path]) {
			RESOURCES::FileWatcher::getInstance()->removeDeferred(path, e);
		}
		fwSubscribersIds.erase(path);
	}
}

void SkeletonAnimationLoader::UpdateFileWatchResource(const std::string& configPath, const std::string& actualPath, std::weak_ptr<SKELETON::Animation> weakRes, std::shared_ptr<SKELETON::Skeleton> skeleton) {
	if (auto animation = weakRes.lock()) {
		std::string loadPath = actualPath;
		bool additive = false;
		SKELETON::Animation* additiveRef = nullptr;
		std::shared_ptr<SKELETON::Animation> additiveRefPtr;

		if (!configPath.empty() && configPath != actualPath) {
			auto content = ServiceManager::Get<FileSystem>().getFile(configPath)->readStr();
			auto resRes = UTILS::FromJsonStr<RENDER::AnimationResource>(content);
			if (resRes.isOk()) {
				auto _res = resRes.unwrap();
				_res.path = configPath;
				sResourceCache[configPath] = _res;
				loadPath = _res.animationPath;
				additive = _res.additive;

				if (!_res.additiveReference.empty()) {
					additiveRefPtr = ServiceManager::Get<SkeletonAnimationLoader>().loadResource(_res.additiveReference);
					if (additiveRefPtr) {
						additiveRef = additiveRefPtr.get();
					}
				}
			}
		}

		Reload(*animation, loadPath, skeleton, additive, additiveRef);
	}
}

void SkeletonAnimationLoader::AddFileWatchSubscribe(const std::string& configPath, const std::string& actualPath, std::weak_ptr<SKELETON::Animation> weakRes, std::shared_ptr<SKELETON::Skeleton> skeleton) {
	auto fwCb = [configPath, actualPath, weakRes, skeleton](RESOURCES::FileWatcher::FileStatus status) {
		switch (status) {
		case RESOURCES::FileWatcher::FileStatus::MODIFIED: {
			UnsubscribeFileWatch(configPath);
			UpdateFileWatchResource(configPath, actualPath, weakRes, skeleton);
			break;
		}
		case RESOURCES::FileWatcher::FileStatus::DEL:
		case RESOURCES::FileWatcher::FileStatus::CREATE:
			break;
		}
	};

	auto saveCb = [configPath](auto e) {
		fwSubscribersIds[configPath].push_back(e);
	};

	if (!configPath.empty() && configPath != actualPath) {
		RESOURCES::FileWatcher::getInstance()->addDeferred(configPath, fwCb, saveCb);
	}

	auto meshSaveCb = [actualPath](auto e) {
		fwSubscribersIds[actualPath].push_back(e);
	};
	RESOURCES::FileWatcher::getInstance()->addDeferred(actualPath, fwCb, meshSaveCb);
}

} // namespace IKIGAI::RESOURCES

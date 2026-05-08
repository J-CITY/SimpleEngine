#include "skeletonManager.h"
#include "serviceManager.h"
#include <resourceModule/fileSystem/fileSystem.h>
#include "skeletalModule/skeleton.h"
#include "utilsModule/assertion.h"
#include "utilsModule/jsonLoader.h"
#include "fileWatcher.h"

namespace IKIGAI::RESOURCES {

AssimpParser SkeletonLoader::_ASSIMP;

ResourcePtr<SKELETON::Skeleton> SkeletonLoader::CreateFromFile(const std::string& path) {
	auto skeleton = std::make_shared<SKELETON::Skeleton>();
	auto data = ServiceManager::Get<FileSystem>().getFile(path)->read();
	if (data.empty()) {
		ASSERT(std::string("SkeletonLoader::CreateFromFile: empty file: " + path).c_str());
		return nullptr;
	}
	if (!_ASSIMP.LoadSkeleton(path, data, *skeleton)) {
		ASSERT(std::string("SkeletonLoader::CreateFromFile: failed to parse: " + path).c_str());
		return nullptr;
	}
	
	// Default call without config
	AddFileWatchSubscribe(path, path, skeleton);
	return skeleton;
}

ResourcePtr<SKELETON::Skeleton> SkeletonLoader::CreateFromResource(const std::string& path) {
	RENDER::SkeletonResource res;
	if (auto it = sResourceCache.find(path); it != sResourceCache.end()) {
		res = it->second;
	} else {
		auto content = ServiceManager::Get<FileSystem>().getFile(path)->readStr();
		auto parsed = UTILS::FromJsonStr<RENDER::SkeletonResource>(content);
		if (parsed.isErr()) {
			ASSERT(std::string("SkeletonLoader::CreateFromResource: can't parse: " + path).c_str());
			return nullptr;
		}
		res = parsed.unwrap();
		res.path = path;
		sResourceCache[path] = res;
	}
	
	auto skeleton = CreateFromFile(res.skeletonPath);
	if (skeleton) {
		AddFileWatchSubscribe(path, res.skeletonPath, skeleton);
	}
	return skeleton;
}

ResourcePtr<SKELETON::Skeleton> SkeletonLoader::createResource(const std::string& path) {
	if (path.ends_with(".skeleton")) {
		return CreateFromResource(path);
	}
	return CreateFromFile(path);
}

ResourcePtr<SKELETON::Skeleton> SkeletonLoader::createResource(const std::string& path, ELoadingType type) {
	return createResource(path, type, std::any());
}

ResourcePtr<SKELETON::Skeleton> SkeletonLoader::createResource(const std::string& path, ELoadingType type, std::any /*data*/) {
	if (type == ELoadingType::FILE) {
		return CreateFromFile(path);
	}
	if (type == ELoadingType::RESOURCE) {
		return CreateFromResource(path);
	}
	return createResource(path);
}

void SkeletonLoader::Reload(SKELETON::Skeleton& skeleton, const std::string& path) {
	auto newSkeleton = CreateFromFile(path);
	if (newSkeleton) {
		skeleton.setNumJoints(newSkeleton->getNumJolts());
		skeleton.joints() = newSkeleton->joints();
	}
}

void SkeletonLoader::UnsubscribeFileWatch(const std::string& path) {
	if (fwSubscribersIds.contains(path)) {
		for (auto& e : fwSubscribersIds[path]) {
			RESOURCES::FileWatcher::getInstance()->removeDeferred(path, e);
		}
		fwSubscribersIds.erase(path);
	}
}

void SkeletonLoader::UpdateFileWatchResource(const std::string& configPath, const std::string& actualPath, std::weak_ptr<SKELETON::Skeleton> weakRes) {
	if (auto skeleton = weakRes.lock()) {
		std::string loadPath = actualPath;

		if (!configPath.empty() && configPath != actualPath) {
			auto content = ServiceManager::Get<FileSystem>().getFile(configPath)->readStr();
			auto resRes = UTILS::FromJsonStr<RENDER::SkeletonResource>(content);
			if (resRes.isOk()) {
				auto _res = resRes.unwrap();
				_res.path = configPath;
				sResourceCache[configPath] = _res;
				loadPath = _res.skeletonPath;
			}
		}

		Reload(*skeleton, loadPath);
	}
}

void SkeletonLoader::AddFileWatchSubscribe(const std::string& configPath, const std::string& actualPath, std::weak_ptr<SKELETON::Skeleton> weakRes) {
	auto fwCb = [configPath, actualPath, weakRes](RESOURCES::FileWatcher::FileStatus status) {
		switch (status) {
		case RESOURCES::FileWatcher::FileStatus::MODIFIED: {
			UnsubscribeFileWatch(configPath);
			UpdateFileWatchResource(configPath, actualPath, weakRes);
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

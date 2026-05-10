#include "skeletonStateGraphManager.h"
#include "serviceManager.h"
#include "fileWatcher.h"
#include "utilsModule/jsonLoader.h"
#include "utilsModule/log/loggerDefine.h"

namespace IKIGAI::RESOURCES {
	//TODO: add allocator and custom deleter support
	//TODO: universal work with filewatch
	//TODO: fix work with sResourceCache

	ResourcePtr<SKELETON::SkeletalStateGraph> SkeletonStateGraphLoader::CreateFromResource(const std::string& path) {
		if (!sResourceCache.contains(path)) {
			auto res = UTILS::FromJson<RENDER::SkeletonStateGraphResource>(path);
			if (res.isOk()) {
				sResourceCache[path] = res.unwrap();
			} else {
				LOG_ERROR << "Failed to parse SkeletonStateGraphResource: " << path;
				return nullptr;
			}
		}
		return std::make_shared<SKELETON::SkeletalStateGraph>(sResourceCache[path]);
	}

	void SkeletonStateGraphLoader::Reload(SKELETON::SkeletalStateGraph& graph, const std::string& path) {
		auto res = UTILS::FromJson<RENDER::SkeletonStateGraphResource>(path);
		if (res.isOk()) {
			sResourceCache[path] = res.unwrap();
			graph.resource = res.unwrap();
		}
	}

	void SkeletonStateGraphLoader::UpdateFileWatchResource(const std::string& path, std::weak_ptr<SKELETON::SkeletalStateGraph> weakRes) {
		if (auto res = weakRes.lock()) {
			Reload(*res, path);
		}
	}

	void SkeletonStateGraphLoader::AddFileWatchSubscribe(const std::string& path, std::weak_ptr<SKELETON::SkeletalStateGraph> weakRes) {
		auto id = ServiceManager::Get<FileWatcher>().add(path, [path, weakRes](RESOURCES::FileWatcher::FileStatus status) {
			switch (status) {
			case RESOURCES::FileWatcher::FileStatus::MODIFIED: {
				UpdateFileWatchResource(path, weakRes);
				break;
			}
			case RESOURCES::FileWatcher::FileStatus::DEL:
			case RESOURCES::FileWatcher::FileStatus::CREATE:
				break;
			}
		});
		fwSubscribersIds[path].push_back(id);
	}

	void SkeletonStateGraphLoader::UnsubscribeFileWatch(const std::string& path) {
		if (fwSubscribersIds.contains(path)) {
			for (auto id : fwSubscribersIds[path]) {
				ServiceManager::Get<FileWatcher>().remove(path, id);
			}
			fwSubscribersIds.erase(path);
		}
	}

	ResourcePtr<SKELETON::SkeletalStateGraph> SkeletonStateGraphLoader::createResource(const std::string& path) {
		return createResource(path, ELoadingType::RESOURCE, std::any());
	}

	ResourcePtr<SKELETON::SkeletalStateGraph> SkeletonStateGraphLoader::createResource(const std::string& path, ELoadingType type) {
		return createResource(path, type, std::any());
	}

	ResourcePtr<SKELETON::SkeletalStateGraph> SkeletonStateGraphLoader::createResource(const std::string& path, ELoadingType type, std::any data) {
		if (type == ELoadingType::RESOURCE) {
			auto graph = CreateFromResource(path);
			AddFileWatchSubscribe(path, graph);
			return graph;
		}
		return nullptr;
	}
}

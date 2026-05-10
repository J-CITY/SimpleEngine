#pragma once

#include "resourceManager.h"
#include "skeletalModule/skeletalStateGraph.h"
#include "utilsModule/event.h"
#include "utilsModule/idGenerator.h"

namespace IKIGAI::RESOURCES {
	class SkeletonStateGraphLoader : public ResourceManager<SKELETON::SkeletalStateGraph> {
	public:
		static ResourcePtr<SKELETON::SkeletalStateGraph> CreateFromResource(const std::string& path);

	private:
		// File watching
		static void Reload(SKELETON::SkeletalStateGraph& graph, const std::string& path);
		static void UpdateFileWatchResource(const std::string& path, std::weak_ptr<SKELETON::SkeletalStateGraph> weakRes);
		static void AddFileWatchSubscribe(const std::string& path, std::weak_ptr<SKELETON::SkeletalStateGraph> weakRes);
		static void UnsubscribeFileWatch(const std::string& path);

		inline static std::unordered_map<std::string, std::vector<IdGenerator<EVENT::Event<>>::ID>> fwSubscribersIds;
		inline static std::unordered_map<std::string, RENDER::SkeletonStateGraphResource> sResourceCache;

		ResourcePtr<SKELETON::SkeletalStateGraph> createResource(const std::string& path) override;
		ResourcePtr<SKELETON::SkeletalStateGraph> createResource(const std::string& path, ELoadingType type) override;
		ResourcePtr<SKELETON::SkeletalStateGraph> createResource(const std::string& path, ELoadingType type, std::any data) override;
	};
}

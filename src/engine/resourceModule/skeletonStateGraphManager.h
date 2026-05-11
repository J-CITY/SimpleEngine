#pragma once

#include "resourceManager.h"
#include "renderModule/backends/interface/driverInterface.h"
#include "skeletalModule/skeletalStateGraph.h"
#include "utilsModule/event.h"
#include "utilsModule/idGenerator.h"

namespace IKIGAI::RESOURCES {
	class SkeletonStateGraphLoader : public ResourceManager<SKELETON::SkeletalStateGraph, RENDER::SkeletonStateGraphResource> {
	public:
		using ResourceDeleter = std::function<void(SKELETON::SkeletalStateGraph*)>;
		static ResourcePtr<SKELETON::SkeletalStateGraph> CreateFromResource(const std::string& path, UTILS::IAllocator* allocator = nullptr, ResourceDeleter deleter = nullptr);
		static ResourcePtr<SKELETON::SkeletalStateGraph> CreateFromResource(const RENDER::SkeletonStateGraphResource& config, UTILS::IAllocator* allocator = nullptr, ResourceDeleter deleter = nullptr);

	private:
		ResourcePtr<SKELETON::SkeletalStateGraph> createResource(const std::string& path) override;
		ResourcePtr<SKELETON::SkeletalStateGraph> createResource(const std::string& path, ELoadingType type) override;
		ResourcePtr<SKELETON::SkeletalStateGraph> createResource(const std::string& path, ELoadingType type, std::any data) override;

		bool reloadResource(std::weak_ptr<SKELETON::SkeletalStateGraph> weakRes, const std::string& path) override;
	};
}

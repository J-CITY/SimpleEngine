#pragma once
#include <string>
#include <any>

#include "resourceManager.h"
#include "parser/assimpParser.h"
#include "renderModule/backends/interface/resourceStruct.h"
#include "utilsModule/event.h"
#include "utilsModule/idGenerator.h"
#include "utilsModule/memoryAlloc.h"

namespace IKIGAI::SKELETON {
	class Skeleton;
}

namespace IKIGAI::RESOURCES {
	class SkeletonLoader : public ResourceManager<SKELETON::Skeleton, RENDER::SkeletonResource> {
	public:
		using ResourceDeleter = std::function<void(SKELETON::Skeleton*)>;
		/// Load directly from a mesh file (.fbx, .dae, etc.)
		static ResourcePtr<SKELETON::Skeleton> CreateFromFile(const std::string& path, UTILS::IAllocator* allocator = nullptr, ResourceDeleter deleter = nullptr);

		/// Load via JSON descriptor (.skeleton file referencing SkeletonResource)
		static ResourcePtr<SKELETON::Skeleton> CreateFromResource(const std::string& path, UTILS::IAllocator* allocator = nullptr, ResourceDeleter deleter = nullptr);

		static ResourcePtr<SKELETON::Skeleton> CreateFromResource(const RENDER::SkeletonResource& res, UTILS::IAllocator* allocator = nullptr, ResourceDeleter deleter = nullptr);

	private:

		static AssimpParser _ASSIMP;

		ResourcePtr<SKELETON::Skeleton> createResource(const std::string& path) override;
		ResourcePtr<SKELETON::Skeleton> createResource(const std::string& path, ELoadingType type) override;
		ResourcePtr<SKELETON::Skeleton> createResource(const std::string& path, ELoadingType type, std::any data) override;

		bool reloadResource(std::weak_ptr<SKELETON::Skeleton> weakRes, const std::string& path) override;
	};
}

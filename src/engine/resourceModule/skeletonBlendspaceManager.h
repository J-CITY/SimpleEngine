#pragma once
#include <string>
#include <any>
#include <memory>
#include <variant>

#include "resourceManager.h"
#include "renderModule/backends/interface/resourceStruct.h"
#include "skeletalModule/blendspace.h"
#include "utilsModule/event.h"
#include "utilsModule/idGenerator.h"
#include "utilsModule/memoryAlloc.h"

namespace IKIGAI::SKELETON {
	class Blendspace1D;
	class Blendspace2D;
}

namespace IKIGAI::RESOURCES {
	class SkeletonBlendspaceLoader : public ResourceManager<SKELETON::BlendspaceInterface, RENDER::SkeletonBlendspace> {
	public:
		static ResourcePtr<SKELETON::BlendspaceInterface> CreateFromResource(const std::string& path, UTILS::IAllocator* allocator = nullptr, ResourceDeleter deleter = nullptr);

	private:

		static ResourcePtr<SKELETON::BlendspaceInterface> CreateBlendspace1D(const RENDER::SkeletonBlendspace& desc, UTILS::IAllocator* allocator = nullptr, ResourceDeleter deleter = nullptr);
		static ResourcePtr<SKELETON::BlendspaceInterface> CreateBlendspace2D(const RENDER::SkeletonBlendspace& desc, UTILS::IAllocator* allocator = nullptr, ResourceDeleter deleter = nullptr);

		ResourcePtr<SKELETON::BlendspaceInterface> createResource(const std::string& path) override;
		ResourcePtr<SKELETON::BlendspaceInterface> createResource(const std::string& path, ELoadingType type) override;
		ResourcePtr<SKELETON::BlendspaceInterface> createResource(const std::string& path, ELoadingType type, std::any data) override;


		bool reloadResource(std::weak_ptr<SKELETON::BlendspaceInterface> weakRes, const std::string& path) override;
	};
}

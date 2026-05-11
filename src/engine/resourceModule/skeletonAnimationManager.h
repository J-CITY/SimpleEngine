#pragma once
#include <string>
#include <any>
#include <memory>

#include "resourceManager.h"
#include "parser/assimpParser.h"
#include "renderModule/backends/interface/resourceStruct.h"
#include "utilsModule/event.h"
#include "utilsModule/idGenerator.h"
#include "utilsModule/memoryAlloc.h"

namespace IKIGAI::SKELETON {
	class Skeleton;
	struct Animation;
}

namespace IKIGAI::RESOURCES {

	/// Context passed via std::any when loading through ResourcePackManager
	struct AnimationLoadContext {
		std::shared_ptr<SKELETON::Skeleton> skeleton;
		bool additive = false;
		std::string additiveReferencePath; // path to already-loaded Animation resource (optional)
	};

	class SkeletonAnimationLoader : public ResourceManager<SKELETON::Animation, RENDER::AnimationResource> {
	public:
		using ResourceDeleter = std::function<void(SKELETON::Animation*)>;
		/// Load directly from a mesh/animation file (.fbx, .dae, etc.)
		static ResourcePtr<SKELETON::Animation> CreateFromFile(
			const std::string& path,
			std::shared_ptr<SKELETON::Skeleton> skeleton,
			bool additive = false,
			SKELETON::Animation* additiveReference = nullptr, UTILS::IAllocator* allocator = nullptr, ResourceDeleter deleter = nullptr);

		/// Load via JSON descriptor (.animation file referencing AnimationResource)
		static ResourcePtr<SKELETON::Animation> CreateFromResource(
			const std::string& path,
			std::shared_ptr<SKELETON::Skeleton> skeleton, UTILS::IAllocator* allocator = nullptr, ResourceDeleter deleter = nullptr);

		static ResourcePtr<SKELETON::Animation> CreateFromResource(
			const RENDER::AnimationResource& config,
			std::shared_ptr<SKELETON::Skeleton> skeleton, UTILS::IAllocator* allocator = nullptr, ResourceDeleter deleter = nullptr);

	private:
		static AssimpParser _ASSIMP;

		ResourcePtr<SKELETON::Animation> createResource(const std::string& path) override;
		ResourcePtr<SKELETON::Animation> createResource(const std::string& path, ELoadingType type) override;
		// data must contain AnimationLoadContext
		ResourcePtr<SKELETON::Animation> createResource(const std::string& path, ELoadingType type, std::any data) override;

		bool reloadResource(std::weak_ptr<SKELETON::Animation> weakRes, const std::string& path) override;
	};
}

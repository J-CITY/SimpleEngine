#pragma once
#include <string>
#include <any>
#include <memory>

#include "resourceManager.h"
#include "parser/assimpParser.h"
#include "renderModule/backends/interface/resourceStruct.h"
#include "utilsModule/event.h"
#include "utilsModule/idGenerator.h"

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

	class SkeletonAnimationLoader : public ResourceManager<SKELETON::Animation> {
	public:
		/// Load directly from a mesh/animation file (.fbx, .dae, etc.)
		static ResourcePtr<SKELETON::Animation> CreateFromFile(
			const std::string& path,
			std::shared_ptr<SKELETON::Skeleton> skeleton,
			bool additive = false,
			SKELETON::Animation* additiveReference = nullptr);

		/// Load via JSON descriptor (.animation file referencing AnimationResource)
		static ResourcePtr<SKELETON::Animation> CreateFromResource(
			const std::string& path,
			std::shared_ptr<SKELETON::Skeleton> skeleton);

	private:
		// File watching
		static void Reload(SKELETON::Animation& animation, const std::string& path, std::shared_ptr<SKELETON::Skeleton> skeleton, bool additive, SKELETON::Animation* additiveReference);
		static void UpdateFileWatchResource(const std::string& configPath, const std::string& actualPath, std::weak_ptr<SKELETON::Animation> weakRes, std::shared_ptr<SKELETON::Skeleton> skeleton);
		static void AddFileWatchSubscribe(const std::string& configPath, const std::string& actualPath, std::weak_ptr<SKELETON::Animation> weakRes, std::shared_ptr<SKELETON::Skeleton> skeleton);
		static void UnsubscribeFileWatch(const std::string& path);

		inline static std::unordered_map<std::string, std::vector<IdGenerator<EVENT::Event<>>::ID>> fwSubscribersIds;

		static AssimpParser _ASSIMP;
		inline static std::unordered_map<std::string, RENDER::AnimationResource> sResourceCache;

		ResourcePtr<SKELETON::Animation> createResource(const std::string& path) override;
		ResourcePtr<SKELETON::Animation> createResource(const std::string& path, ELoadingType type) override;
		// data must contain AnimationLoadContext
		ResourcePtr<SKELETON::Animation> createResource(const std::string& path, ELoadingType type, std::any data) override;
	};
}

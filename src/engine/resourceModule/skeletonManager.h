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
	class SkeletonLoader : public ResourceManager<SKELETON::Skeleton> {
	public:
		/// Load directly from a mesh file (.fbx, .dae, etc.)
		static ResourcePtr<SKELETON::Skeleton> CreateFromFile(const std::string& path);

		/// Load via JSON descriptor (.skeleton file referencing SkeletonResource)
		static ResourcePtr<SKELETON::Skeleton> CreateFromResource(const std::string& path);

	private:
		// File watching
		static void Reload(SKELETON::Skeleton& skeleton, const std::string& path);
		static void UpdateFileWatchResource(const std::string& configPath, const std::string& actualPath, std::weak_ptr<SKELETON::Skeleton> weakRes);
		static void AddFileWatchSubscribe(const std::string& configPath, const std::string& actualPath, std::weak_ptr<SKELETON::Skeleton> weakRes);
		static void UnsubscribeFileWatch(const std::string& path);

		inline static std::unordered_map<std::string, std::vector<IdGenerator<EVENT::Event<>>::ID>> fwSubscribersIds;

		static AssimpParser _ASSIMP;
		inline static std::unordered_map<std::string, RENDER::SkeletonResource> sResourceCache;

		ResourcePtr<SKELETON::Skeleton> createResource(const std::string& path) override;
		ResourcePtr<SKELETON::Skeleton> createResource(const std::string& path, ELoadingType type) override;
		ResourcePtr<SKELETON::Skeleton> createResource(const std::string& path, ELoadingType type, std::any data) override;
	};
}

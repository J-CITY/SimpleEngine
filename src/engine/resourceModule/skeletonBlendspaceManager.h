#pragma once
#include <string>
#include <any>
#include <memory>
#include <variant>

#include "resourceManager.h"
#include "renderModule/backends/interface/resourceStruct.h"

namespace IKIGAI::SKELETON {
	class Blendspace1D;
	class Blendspace2D;
}

namespace IKIGAI::RESOURCES {

	/// Universal handle for any blendspace type (1D or 2D)
	using BlendspaceVariant = std::variant<
		std::shared_ptr<SKELETON::Blendspace1D>,
		std::shared_ptr<SKELETON::Blendspace2D>
	>;

	class SkeletonBlendspaceLoader : public ResourceManager<BlendspaceVariant> {
	public:
		/// Load via JSON descriptor (.blendspace1d / .blendspace2d file)
		/// The skeleton and animations referenced inside the descriptor are
		/// automatically resolved through ServiceManager.
		static ResourcePtr<BlendspaceVariant> CreateFromResource(const std::string& path);

	private:
		// File watching
		static void Reload(BlendspaceVariant& variant, const std::string& path);
		static void UpdateFileWatchResource(const std::string& path, std::weak_ptr<BlendspaceVariant> weakRes);
		static void AddFileWatchSubscribe(const std::string& path, std::weak_ptr<BlendspaceVariant> weakRes);
		static void UnsubscribeFileWatch(const std::string& path);

		inline static std::unordered_map<std::string, std::vector<IKIGAI::IdGenerator<EVENT::Event<>>::ID>> fwSubscribersIds;

		inline static std::unordered_map<std::string, RENDER::SkeletonBlendspace1D> sResourceCache1D;
		inline static std::unordered_map<std::string, RENDER::SkeletonBlendspace2D> sResourceCache2D;

		static ResourcePtr<BlendspaceVariant> CreateBlendspace1D(const RENDER::SkeletonBlendspace1D& desc);
		static ResourcePtr<BlendspaceVariant> CreateBlendspace2D(const RENDER::SkeletonBlendspace2D& desc);

		ResourcePtr<BlendspaceVariant> createResource(const std::string& path) override;
		ResourcePtr<BlendspaceVariant> createResource(const std::string& path, ELoadingType type) override;
		ResourcePtr<BlendspaceVariant> createResource(const std::string& path, ELoadingType type, std::any data) override;
	};
}

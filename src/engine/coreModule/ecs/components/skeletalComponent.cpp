#include "skeletalComponent.h"
#include "resourceModule/serviceManager.h"
#include "resourceModule/skeletonManager.h"

namespace IKIGAI::ECS {
	SkeletalComponent::SkeletalComponent(UTILS::Ref<ECS::Object> obj) : ComponentBase(obj) {
		__NAME__ = "SkeletalComponent";
	}

	SkeletalComponent::~SkeletalComponent() = default;

	void SkeletalComponent::setSkeleton(const std::string& path) {
		mSkeletonPath = path;
		if (!path.empty()) {
			mSkeleton = RESOURCES::ServiceManager::Get<RESOURCES::SkeletonLoader>().loadResource(path);
		} else {
			mSkeleton = nullptr;
		}
	}

	void SkeletalComponent::onDeserialize(nlohmann::json& j) {
		if (j.contains("SkeletonPath")) {
			setSkeleton(j["SkeletonPath"].get<std::string>());
		}
	}

	void SkeletalComponent::onSerialize(nlohmann::json& j) {
		j["SkeletonPath"] = mSkeletonPath;
	}
}

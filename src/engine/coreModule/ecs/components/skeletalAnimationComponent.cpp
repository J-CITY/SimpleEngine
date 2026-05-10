#include "skeletalAnimationComponent.h"
#include "skeletalComponent.h"
#include "coreModule/ecs/object.h"
#include "resourceModule/serviceManager.h"
#include "resourceModule/skeletonStateGraphManager.h"
#include "skeletalModule/skeletalStateGraph.h"

namespace IKIGAI::ECS {
	SkeletalAnimationComponent::SkeletalAnimationComponent(UTILS::Ref<ECS::Object> obj) : ComponentBase(obj) {
		__NAME__ = "SkeletalAnimationComponent";
		mEnvironment = std::make_shared<UTILS::Environment>();
	}

	SkeletalAnimationComponent::~SkeletalAnimationComponent() = default;

	void SkeletalAnimationComponent::onUpdate(std::chrono::duration<double> dt) {
		if (mPlayable) {
			mPlayable->update(static_cast<float>(dt.count()));
		}
	}

	void SkeletalAnimationComponent::setPlayable(std::unique_ptr<SKELETON::IAnimationPlayable> playable) {
		mPlayable = std::move(playable);
	}

	void SkeletalAnimationComponent::setStateGraph(const std::string& path) {
		mGraphPath = path;
		if (!path.empty()) {
			auto graphRes = RESOURCES::ServiceManager::Get<RESOURCES::SkeletonStateGraphLoader>().loadResource(path);
			if (graphRes) {
				auto skelComp = obj->getComponent<SkeletalComponent>();
				SKELETON::Skeleton* skeleton = nullptr;
				if (skelComp && skelComp->getSkeleton()) {
					skeleton = skelComp->getSkeleton().get();
				}
				
				mPlayable = std::make_unique<SKELETON::SkeletalStateGraphInstance>(skeleton, graphRes, mEnvironment);
			}
		} else {
			mPlayable = nullptr;
		}
	}

	void SkeletalAnimationComponent::onDeserialize(nlohmann::json& j) {
		if (j.contains("GraphPath")) {
			setStateGraph(j["GraphPath"].get<std::string>());
		}
	}

	void SkeletalAnimationComponent::onSerialize(nlohmann::json& j) {
		j["GraphPath"] = mGraphPath;
	}
}

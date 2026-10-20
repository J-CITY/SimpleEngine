#pragma once

#include "animationBlend.h"
#include "iAnimationPlayable.h"
#include "renderModule/backends/interface/resourceStruct.h"
#include "stateGraphRuntime.h"
#include "utilsModule/environment.h"
#include <memory>
#include <string>
#include <vector>

namespace IKIGAI::SKELETON {
class SkeletalStateGraph {
public:
	RENDER::SkeletonStateGraphResource resource;
	std::shared_ptr<const StateGraphDefinition> definition;
	unsigned revision = 0;

	SkeletalStateGraph() = default;
	explicit SkeletalStateGraph(const RENDER::SkeletonStateGraphResource& res)
		: resource(res) { }
};

class SkeletalStateGraphInstance : public IAnimationPlayable {
public:
	enum class ReloadPolicy { Automatic, OwnerValidated };
	void setReloadPolicy(ReloadPolicy policy) {
		mReloadPolicy = policy;
	}
	SkeletalStateGraphInstance(
		std::shared_ptr<Skeleton> skeleton, std::shared_ptr<SkeletalStateGraph> graph, std::shared_ptr<UTILS::Environment> env);
	~SkeletalStateGraphInstance() override;

	void update(float dt) override;
	Pose* getPose() override;
	std::unique_ptr<IAnimationPlayable> clone() const override;
	StateGraphRuntime* runtime() const {
		return mRuntime.get();
	}

private:
	ReloadPolicy mReloadPolicy = ReloadPolicy::Automatic;
	std::unique_ptr<StateGraphRuntime> mRuntime;
	unsigned mRevision = 0;
	// evaluateCondition moved to ConditionEvaluator
	void changeState(const std::string& stateName, float blendTime);
	std::unique_ptr<IAnimationPlayable> instantiateStatePlayable(const RENDER::AnimStateResource& stateRes);

	std::shared_ptr<Skeleton> mSkeleton;
	std::shared_ptr<SkeletalStateGraph> mGraph;
	std::shared_ptr<UTILS::Environment> mEnvironment;

	std::string mCurrentStateName;
	std::unique_ptr<IAnimationPlayable> mCurrentPlayable;

	// For blending transitions
	std::string mNextStateName;
	std::unique_ptr<IAnimationPlayable> mNextPlayable;
	float mBlendTime = 0.0f;
	float mBlendTimeMax = 0.0f;
	std::unique_ptr<AnimBlend> mTransitionBlend;
	Pose* mLastPose = nullptr;
};
}

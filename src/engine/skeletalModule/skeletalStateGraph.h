#pragma once

#include <string>
#include <vector>
#include <memory>
#include "renderModule/backends/interface/resourceStruct.h"
#include "iAnimationPlayable.h"
#include "utilsModule/environment.h"
#include "animationBlend.h"

namespace IKIGAI::SKELETON {
	class SkeletalStateGraph {
	public:
		RENDER::SkeletonStateGraphResource resource;

		SkeletalStateGraph() = default;
		explicit SkeletalStateGraph(const RENDER::SkeletonStateGraphResource& res) : resource(res) {}
	};

	class SkeletalStateGraphInstance : public IAnimationPlayable {
	public:
		SkeletalStateGraphInstance(Skeleton* skeleton, std::shared_ptr<SkeletalStateGraph> graph, std::shared_ptr<UTILS::Environment> env);
		~SkeletalStateGraphInstance() override;

		void update(float dt) override;
		Pose* getPose() override;
		std::unique_ptr<IAnimationPlayable> clone() const override;

	private:
		// evaluateCondition moved to ConditionEvaluator
		void changeState(const std::string& stateName, float blendTime);
		std::unique_ptr<IAnimationPlayable> instantiateStatePlayable(const RENDER::AnimStateResource& stateRes);

		Skeleton* mSkeleton;
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

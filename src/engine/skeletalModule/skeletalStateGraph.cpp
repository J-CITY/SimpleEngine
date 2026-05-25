#include "skeletalStateGraph.h"

#include "animationInstance.h"
#include "blendspace.h"
#include "resourceModule/serviceManager.h"
#include "resourceModule/skeletonAnimationManager.h"
#include "resourceModule/skeletonBlendspaceManager.h"
#include "utilsModule/conditionEvaluator.h"

namespace IKIGAI::SKELETON {

	SkeletalStateGraphInstance::SkeletalStateGraphInstance(std::shared_ptr<Skeleton> skeleton, std::shared_ptr<SkeletalStateGraph> graph, std::shared_ptr<UTILS::Environment> env)
		: mSkeleton(skeleton), mGraph(graph), mEnvironment(env) {
		mTransitionBlend = std::make_unique<AnimBlend>(skeleton);

		if (mGraph && !mGraph->resource.initialState.empty()) {
			changeState(mGraph->resource.initialState, 0.0f);
		}
	}

	SkeletalStateGraphInstance::~SkeletalStateGraphInstance() = default;

	void SkeletalStateGraphInstance::update(float dt) {
		if (!mGraph) return;

		// 1. Check transitions from current state
		if (!mCurrentStateName.empty() && mBlendTime <= 0.0f) { // Only check if not already blending
			const RENDER::AnimStateResource* currentStateRes = nullptr;
			for (const auto& state : mGraph->resource.states) {
				if (state.name == mCurrentStateName) {
					currentStateRes = &state;
					break;
				}
			}

			if (currentStateRes) {
				for (const auto& transition : currentStateRes->transitions) {
					bool conditionsMet = true;
					for (const auto& cond : transition.conditions) {
						if (!UTILS::ConditionEvaluator::Evaluate(cond, *mEnvironment)) {
							conditionsMet = false;
							break;
						}
					}

					if (conditionsMet) {
						changeState(transition.targetState, transition.blendTime);
						break; // Take the first valid transition
					}
				}
			}
		}

		// 2. Update current states and blending
		if (mCurrentPlayable) {
			// If it's a blendspace, update its variables from environment
			if (auto bs1d = dynamic_cast<Blendspace1DInstance*>(mCurrentPlayable.get())) {
				// We need the state resource to get variable names
				const RENDER::AnimStateResource* s = nullptr;
				for (const auto& state : mGraph->resource.states) {
					if (state.name == mCurrentStateName) { s = &state; break; }
				}
				if (s && !s->blendXVariable.empty()) {
					if (auto val = mEnvironment->getValue(s->blendXVariable); std::holds_alternative<float>(val)) {
						bs1d->setValue(std::get<float>(val));
					}
				}
			}
			else if (auto bs2d = dynamic_cast<Blendspace2DInstance*>(mCurrentPlayable.get())) {
				const RENDER::AnimStateResource* s = nullptr;
				for (const auto& state : mGraph->resource.states) {
					if (state.name == mCurrentStateName) { s = &state; break; }
				}
				if (s) {
					if (!s->blendXVariable.empty()) {
						if (auto val = mEnvironment->getValue(s->blendXVariable); std::holds_alternative<float>(val)) {
							bs2d->setXValue(std::get<float>(val));
						}
					}
					if (!s->blendYVariable.empty()) {
						if (auto val = mEnvironment->getValue(s->blendYVariable); std::holds_alternative<float>(val)) {
							bs2d->setYValue(std::get<float>(val));
						}
					}
				}
			}

			mCurrentPlayable->update(dt);
			mLastPose = mCurrentPlayable->getPose();
		}

		if (mBlendTime > 0.0f && mNextPlayable) {
			mNextPlayable->update(dt);
			Pose* nextPose = mNextPlayable->getPose();
			
			mBlendTime -= dt;
			
			float factor = 1.0f - (mBlendTime / mBlendTimeMax);
			if (factor > 1.0f) factor = 1.0f;
			if (factor < 0.0f) factor = 0.0f;

			mLastPose = mTransitionBlend->blend(mLastPose, nextPose, factor);

			if (mBlendTime <= 0.0f) {
				mCurrentStateName = mNextStateName;
				mCurrentPlayable = std::move(mNextPlayable);
				mNextStateName.clear();
			}
		}
	}

	Pose* SkeletalStateGraphInstance::getPose() {
		return mLastPose;
	}
	// evaluateCondition removed

	void SkeletalStateGraphInstance::changeState(const std::string& stateName, float blendTime) {
		const RENDER::AnimStateResource* nextStateRes = nullptr;
		for (const auto& state : mGraph->resource.states) {
			if (state.name == stateName) {
				nextStateRes = &state;
				break;
			}
		}

		if (!nextStateRes) return;

		auto newPlayable = instantiateStatePlayable(*nextStateRes);
		if (!newPlayable) return;

		if (mCurrentPlayable && blendTime > 0.0f) {
			mNextStateName = stateName;
			mNextPlayable = std::move(newPlayable);
			mBlendTimeMax = blendTime;
			mBlendTime = blendTime;
		} else {
			mCurrentStateName = stateName;
			mCurrentPlayable = std::move(newPlayable);
			mBlendTime = 0.0f;
		}
	}

	std::unique_ptr<IAnimationPlayable> SkeletalStateGraphInstance::instantiateStatePlayable(const RENDER::AnimStateResource& stateRes) {
		if (stateRes.type == RENDER::AnimStateType::ANIMATION) {
			auto animRes = RESOURCES::ServiceManager::Get<RESOURCES::SkeletonAnimationLoader>().loadResource(stateRes.resourcePath);
			if (animRes) {
				return std::make_unique<AnimSample>(mSkeleton, animRes);
			}
		} else if (stateRes.type == RENDER::AnimStateType::BLENDSPACE_1D || stateRes.type == RENDER::AnimStateType::BLENDSPACE_2D) {
			auto bsRes = RESOURCES::ServiceManager::Get<RESOURCES::SkeletonBlendspaceLoader>().loadResource(stateRes.resourcePath);
			if (bsRes) {
				if (stateRes.type == RENDER::AnimStateType::BLENDSPACE_1D) {
					return std::make_unique<Blendspace1DInstance>(std::static_pointer_cast<Blendspace1D>(bsRes));
				}
				else {
					return std::make_unique<Blendspace2DInstance>(std::static_pointer_cast<Blendspace2D>(bsRes));
				}
			}
		}
		return nullptr;
	}

	std::unique_ptr<IAnimationPlayable> SkeletalStateGraphInstance::clone() const {
		return std::make_unique<SkeletalStateGraphInstance>(mSkeleton, mGraph, mEnvironment);
	}
}

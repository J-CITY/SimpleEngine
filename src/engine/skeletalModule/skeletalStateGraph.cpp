#include "skeletalStateGraph.h"

#include "animationInstance.h"
#include "blendspace.h"
#include "resourceModule/serviceManager.h"
#include "resourceModule/skeletonAnimationManager.h"
#include "resourceModule/skeletonBlendspaceManager.h"
#include "utilsModule/conditionEvaluator.h"
#include <iostream>

namespace IKIGAI::SKELETON {

SkeletalStateGraphInstance::SkeletalStateGraphInstance(
	std::shared_ptr<Skeleton> skeleton, std::shared_ptr<SkeletalStateGraph> graph, std::shared_ptr<UTILS::Environment> env)
	: mSkeleton(skeleton)
	, mGraph(graph)
	, mEnvironment(env) {
	if (mGraph && mGraph->definition) {
		const auto expected = mGraph->definition->skeleton;
		if (!mSkeleton || mSkeleton->getNumJolts() != expected->getNumJolts()) {
			throw std::runtime_error("State graph skeleton differs from visual skeleton");
		}
		for (size_t i = 0; i < mSkeleton->joints().size(); ++i) {
			if (mSkeleton->joints()[i].name != expected->joints()[i].name
				|| mSkeleton->joints()[i].parentIndex != expected->joints()[i].parentIndex) {
				throw std::runtime_error("State graph joint layout differs from visual skeleton");
			}
		}
		mRuntime = std::make_unique<StateGraphRuntime>(mGraph->definition, mEnvironment);
		mRevision = mGraph->revision;
		return;
	}
	mTransitionBlend = std::make_unique<AnimBlend>(skeleton);

	if (mGraph && !mGraph->resource.initialState.empty()) {
		changeState(mGraph->resource.initialState, 0.0f);
		update(0);
	}
}

SkeletalStateGraphInstance::~SkeletalStateGraphInstance() = default;

void SkeletalStateGraphInstance::update(float dt) {
	if (mRuntime) {
		if (mReloadPolicy == ReloadPolicy::Automatic && mRevision != mGraph->revision && !mRuntime->metadata().contains("Action")) {
			std::string error;
			if (!mRuntime->replace(mGraph->definition, error)) {
				std::cerr << "State graph reload rejected: " << error << std::endl;
			}
			else {
				std::cout << "STATE_GRAPH_RELOAD_OK state=" << mRuntime->state() << std::endl;
			}
			mRevision = mGraph->revision;
		}
		mRuntime->update(dt);
		return;
	}
	if (!mGraph) {
		return;
	}

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

		auto bind = [&](IAnimationPlayable* playable, const std::string& name) {
			for (const auto& state : mGraph->resource.states) {
				if (state.name == name) {
					auto read = [&](const std::string& variable) {
						// Version 1 blendspaces historically used zero for an unbound
						// input. Version 2 validates all parameter slots at load time.
						if (!mEnvironment->hasVariable(variable)) {
							return 0.f;
						}
						return std::visit([](auto value) { return float(value); }, mEnvironment->getValue(variable));
					};
					if (auto bs = dynamic_cast<Blendspace1DInstance*>(playable)) {
						if (!state.blendXVariable.empty()) {
							bs->setValue(read(state.blendXVariable));
						}
					}
					else if (auto bs = dynamic_cast<Blendspace2DInstance*>(playable)) {
						if (!state.blendXVariable.empty()) {
							bs->setXValue(read(state.blendXVariable));
						}
						if (!state.blendYVariable.empty()) {
							bs->setYValue(read(state.blendYVariable));
						}
					}
					break;
				}
			}
		};
		bind(mCurrentPlayable.get(), mCurrentStateName);
		if (mNextPlayable) {
			bind(mNextPlayable.get(), mNextStateName);
		}

		mCurrentPlayable->update(dt);
		mLastPose = mCurrentPlayable->getPose();
	}

	if (mBlendTime > 0.0f && mNextPlayable) {
		mNextPlayable->update(dt);
		Pose* nextPose = mNextPlayable->getPose();

		mBlendTime -= dt;

		float factor = 1.0f - (mBlendTime / mBlendTimeMax);
		if (factor > 1.0f) {
			factor = 1.0f;
		}
		if (factor < 0.0f) {
			factor = 0.0f;
		}

		mLastPose = mTransitionBlend->blend(mLastPose, nextPose, factor);

		if (mBlendTime <= 0.0f) {
			mCurrentStateName = mNextStateName;
			mCurrentPlayable = std::move(mNextPlayable);
			mNextStateName.clear();
		}
	}
}

Pose* SkeletalStateGraphInstance::getPose() {
	if (mRuntime) {
		return mRuntime->getPose();
	}
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

	if (!nextStateRes) {
		return;
	}

	auto newPlayable = instantiateStatePlayable(*nextStateRes);
	if (!newPlayable) {
		return;
	}

	if (mCurrentPlayable && blendTime > 0.0f) {
		mNextStateName = stateName;
		mNextPlayable = std::move(newPlayable);
		mBlendTimeMax = blendTime;
		mBlendTime = blendTime;
	}
	else {
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
	}
	else if (stateRes.type == RENDER::AnimStateType::BLENDSPACE_1D || stateRes.type == RENDER::AnimStateType::BLENDSPACE_2D) {
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
	auto env = std::make_shared<UTILS::Environment>(mEnvironment->snapshot());
	auto copy = std::make_unique<SkeletalStateGraphInstance>(mSkeleton, mGraph, env);
	copy->mReloadPolicy = mReloadPolicy;
	copy->mRevision = mRevision;
	if (mRuntime) {
		copy->mRuntime = std::make_unique<StateGraphRuntime>(*mRuntime);
		copy->mEnvironment = copy->mRuntime->environment();
		return copy;
	}
	copy->mCurrentStateName = mCurrentStateName;
	copy->mNextStateName = mNextStateName;
	copy->mCurrentPlayable = mCurrentPlayable ? mCurrentPlayable->clone() : nullptr;
	copy->mNextPlayable = mNextPlayable ? mNextPlayable->clone() : nullptr;
	copy->mBlendTime = mBlendTime;
	copy->mBlendTimeMax = mBlendTimeMax;
	copy->mLastPose = copy->mCurrentPlayable ? copy->mCurrentPlayable->getPose() : nullptr;
	if (copy->mNextPlayable && copy->mBlendTime > 0) {
		copy->mLastPose = copy->mTransitionBlend->blend(copy->mLastPose, copy->mNextPlayable->getPose(), 1 - copy->mBlendTime / copy->mBlendTimeMax);
	}
	return copy;
}
}

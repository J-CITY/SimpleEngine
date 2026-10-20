#pragma once
#include "animationBlend.h"
#include "animationInstance.h"
#include "utilsModule/environment.h"
#include "utilsModule/expressions/expression.h"
#include "utilsModule/expressions/stateMachine.h"
#include <functional>
#include <map>
#include <nlohmann/json.hpp>

namespace IKIGAI::SKELETON {
// Immutable, validated and preloaded. Instances own all clocks and samplers.
struct StateGraphDefinition {
	using Json = nlohmann::json;
	Json document;
	Json executable;
	DATA::Symbols symbols;
	std::vector<DATA::Expression> expressions;
	struct Binding {
		size_t slot;
		DATA::Expression expression;
		float response;
	};
	std::vector<Binding> bindings;
	size_t parameterCount = 0, builtinStart = 0;
	std::vector<std::string> parameters;
	DATA::StateMachineDefinition machine;
	std::shared_ptr<Skeleton> skeleton;
	std::map<std::string, std::shared_ptr<Animation>> clips;
	std::vector<std::string> dependencies;
	static std::shared_ptr<const StateGraphDefinition> compile(
		const Json&, std::shared_ptr<Skeleton>, const std::function<std::shared_ptr<Animation>(const std::string&)>&);
};

class StateGraphRuntime : public DATA::StateMachineInstance {
public:
	using Json = nlohmann::json;
	struct Action {
		std::string state;
		std::shared_ptr<AnimSample> sample;
		Json metadata;
		float duration = 0;
		std::shared_ptr<const StateGraphDefinition> owner;
		explicit operator bool() const {
			return bool(sample);
		}
	};
	StateGraphRuntime(std::shared_ptr<const StateGraphDefinition>, std::shared_ptr<UTILS::Environment>);
	StateGraphRuntime(const StateGraphRuntime&);
	void update(float dt);
	Pose* getPose() {
		return &mOutput;
	}
	const std::string& state() const {
		return mState;
	}
	const std::string& previousState() const {
		return mPreviousState;
	}
	const Json& lastTransition() const {
		return mLastTransition;
	}
	const std::map<std::string, float>& syncPhases() const {
		return mPhases;
	}
	float stateTime() const {
		return mTime;
	}
	float blendWeight() const {
		return mBlendDuration > 0 ? std::min(1.f, mBlendTime / mBlendDuration) : 1.f;
	}
	const Json& definition() const {
		return mDefinition->executable;
	}
	const Json& metadata() const;
	std::shared_ptr<UTILS::Environment> environment() const {
		return mEnvironment;
	}
	Action requestAction(const std::string& channel);
	Action resolveAction(const std::string& channel);
	bool acceptAction(const Action&);
	void reset();
	void setPoseOffset(MATH::Vector3f offset) {
		mPoseOffset = offset;
	}
	bool replace(std::shared_ptr<const StateGraphDefinition>, std::string& error);
	float evaluate(const Json&) const;

private:
	const Json& stateResource(const std::string&) const;
	void enter(const std::string&, float);
	Pose sampleNode(const Json&, float time, float dt);
	float sampleTime(const std::string&, float time) const;
	void updateBindings(float dt);
	std::shared_ptr<const StateGraphDefinition> mDefinition;
	std::shared_ptr<UTILS::Environment> mEnvironment;
	std::map<std::string, std::shared_ptr<AnimSample>> mSamples;
	std::map<std::string, float> mPhases;
	std::map<std::string, float> mSmoothedBindings;
	std::vector<DATA::Value> mExpressionValues;
	std::vector<size_t> mParameterSlots;
	DATA::Value readExpressionSlot(size_t) const;
	std::map<std::string, std::pair<float, float>> mSavedTimes;
	float mPlaybackTime = 0, mPreviousPlaybackTime = 0;
	bool mSnapshotBlend = false;
	Pose mOutput {}, mPreviousPose {};
	Json mLastTransition;
	MATH::Vector3f mPoseOffset {};
	AnimBlend mBlend;
};
}

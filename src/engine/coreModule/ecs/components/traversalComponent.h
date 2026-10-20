#pragma once
#if USE_PHYSICS_3D
#include "coreModule/ecs/componentReference.h"
#include "coreModule/ecs/components/component.h"
#include "physicsModule/physicWorld.h"
#include "resourceModule/descriptors/traversalActions.h"
#include "skeletalModule/animationInstance.h"
#include "skeletalModule/characterAnimation.h"
namespace IKIGAI::ECS {
class TraversalComponent : public ComponentBase {
public:
	struct Descriptor : ComponentBase::Descriptor {
		std::string Type;
		ComponentReference Motor = {};
		std::string Actions = {};
		template <class C> static auto serde(C& c, Descriptor& v) {
			using namespace serde::attribute;
			serde::serde_struct(c, v)
				.field(&Descriptor::Type, "TraversalComponentType")
				.field(&Descriptor::Motor, "Motor", default_ { ComponentReference {} })
				.field(&Descriptor::Actions, "Actions", default_ { std::string {} });
		}
	};
	explicit TraversalComponent(UTILS::Ref<Object>);
	TraversalComponent(UTILS::Ref<Object>, const ComponentBase::Descriptor& d);
	Descriptor getDescriptor() const {
		return settings;
	}
	static auto GetMembers() {
		return std::tuple {};
	}
	Descriptor settings;
	struct Action {
		int id = 0;
		std::shared_ptr<SKELETON::AnimSample> sample;
		RESOURCES::TraversalMotionDescriptor metadata;
		float duration = 0;
		explicit operator bool() const {
			return bool(sample);
		}
	};
	struct Candidate {
		int id;
		int priority;
		DATA::Expression condition;
		std::shared_ptr<SKELETON::Animation> animation;
		RESOURCES::TraversalMotionDescriptor metadata;
		float rate;
	};
	std::vector<Candidate> candidates;
	std::shared_ptr<SKELETON::Skeleton> skeleton;
	SKELETON::CHARACTER::Rig rig;
	bool prepared = false;
	bool traversing = false;
	bool vault = false;
	bool accepted = false;
	bool completed = false;
	bool cancelled = false;
	int actionId = 0;
	MATH::Vector3f traversalStart {};
	MATH::Vector3f traversalRaised {};
	MATH::Vector3f traversalEnd {};
	MATH::Vector3f ledge {};
	MATH::Vector3f originStart {};
	MATH::Vector3f originEnd {};
	MATH::Vector3f visualFeet {};
	MATH::Vector3f poseOffset {};
	PHYSICS::Id traversalObstacle = 0;
	float obstacleHeight = 0;
	float progress = 0;
	float actionDuration = 1;
	float plantPhase = 0;
	float releasePhase = 0;
	Action actionClip;
};
template <> inline std::string GetType<TraversalComponent>() {
	return "class IKIGAI::ECS::TraversalComponent";
}
template <> inline std::string GetComponentName<TraversalComponent>() {
	return "TraversalComponent";
}
}
#endif

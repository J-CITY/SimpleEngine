#pragma once
#include "utilsModule/expressions/expressionDescriptor.h"
#include <optional>
#include <serdepp/serde.hpp>

namespace IKIGAI::RESOURCES {
struct TraversalMotionDescriptor {
	bool Contact = false;
	bool SingleHand = false;
	float Plant = 0.f;
	float Release = 1.f;
	template <class C> static auto serde(C& context, TraversalMotionDescriptor& descriptor) {
		using namespace serde::attribute;
		serde::serde_struct(context, descriptor)
			.field(&TraversalMotionDescriptor::Contact, "Contact", default_ { false })
			.field(&TraversalMotionDescriptor::SingleHand, "SingleHand", default_ { false })
			.field(&TraversalMotionDescriptor::Plant, "Plant", default_ { 0.f })
			.field(&TraversalMotionDescriptor::Release, "Release", default_ { 1.f });
	}
};
struct TraversalRigDescriptor {
	std::string Root;
	std::vector<std::string> Feet;
	std::vector<std::string> Hands;
	float UnitsToMeters = .01f;
	float RootHeight = 100.f;
	float SoleOffset = .03f;
	template <class C> static auto serde(C& context, TraversalRigDescriptor& descriptor) {
		using namespace serde::attribute;
		serde::serde_struct(context, descriptor)
			.field(&TraversalRigDescriptor::Root, "Root")
			.field(&TraversalRigDescriptor::Feet, "Feet")
			.field(&TraversalRigDescriptor::Hands, "Hands")
			.field(&TraversalRigDescriptor::UnitsToMeters, "UnitsToMeters", default_ { .01f })
			.field(&TraversalRigDescriptor::RootHeight, "RootHeight", default_ { 100.f })
			.field(&TraversalRigDescriptor::SoleOffset, "SoleOffset", default_ { .03f });
	}
};
struct TraversalActionDescriptor {
	int Id = 0;
	std::string Name;
	std::string Clip;
	float Rate = 1.f;
	int Priority = 0;
	std::optional<DATA::ExpressionDescriptor> When;
	TraversalMotionDescriptor Motion;
	template <class C> static auto serde(C& context, TraversalActionDescriptor& descriptor) {
		using namespace serde::attribute;
		serde::serde_struct(context, descriptor)
			.field(&TraversalActionDescriptor::Id, "Id")
			.field(&TraversalActionDescriptor::Name, "Name", default_ { std::string {} })
			.field(&TraversalActionDescriptor::Clip, "Clip")
			.field(&TraversalActionDescriptor::Rate, "Rate", default_ { 1.f })
			.field(&TraversalActionDescriptor::Priority, "Priority", default_ { 0 })
			.field(&TraversalActionDescriptor::When, "When")
			.field(&TraversalActionDescriptor::Motion, "Motion", default_ { TraversalMotionDescriptor {} });
	}
};
struct TraversalActionsDescriptor {
	int Version = 1;
	std::string SkeletonPath;
	TraversalRigDescriptor Rig;
	std::vector<TraversalActionDescriptor> Actions;
	template <class C> static auto serde(C& context, TraversalActionsDescriptor& descriptor) {
		serde::serde_struct(context, descriptor)
			.field(&TraversalActionsDescriptor::Version, "Version")
			.field(&TraversalActionsDescriptor::SkeletonPath, "SkeletonPath")
			.field(&TraversalActionsDescriptor::Rig, "Rig")
			.field(&TraversalActionsDescriptor::Actions, "Actions");
	}
};
}

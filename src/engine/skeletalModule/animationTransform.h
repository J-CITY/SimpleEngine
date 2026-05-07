#pragma once

#include "skeleton.h"

namespace IKIGAI::SKELETON {
	class AnimLocalTransform {
	public:
		AnimLocalTransform(Skeleton* skeleton);
		~AnimLocalTransform();
		PoseTransforms* generateTransforms(Pose* pose);

	private:
		MATH::Matrix4f transformFromKeyframe(const Keyframe& keyframe);

	private:
		Skeleton* mSkeleton;
		PoseTransforms mTransforms;
	};

	class AnimGlobalTransform {
	public:
		AnimGlobalTransform(Skeleton* skeleton);
		~AnimGlobalTransform();
		PoseTransforms* generateTransforms(PoseTransforms* localTransforms);

	private:
		Skeleton* mSkeleton;
		PoseTransforms mTransforms;
	};
}

#include "animationTransform.h"

namespace IKIGAI::SKELETON {
	AnimLocalTransform::AnimLocalTransform(Skeleton* skeleton) : mSkeleton(skeleton) {
		for (int i = 0; i < MAXBONES; i++) {
			mTransforms.transforms[i] = MATH::Matrix4f(1.0f);
		}
	}

	AnimLocalTransform::~AnimLocalTransform() = default;

	PoseTransforms* AnimLocalTransform::generateTransforms(Pose* pose) {
		for (uint32_t i = 0; i < mSkeleton->getNumJolts(); i++) {
			mTransforms.transforms[i] = transformFromKeyframe(pose->keyframes[i]);
		}
		return &mTransforms;
	}


	MATH::Matrix4f AnimLocalTransform::transformFromKeyframe(const Keyframe& keyframe) {
		MATH::Matrix4f translation = MATH::Matrix4f::Translation(keyframe.translation);
		MATH::Matrix4f rotation = MATH::QuaternionF::ToMatrix4(keyframe.rotation);
		MATH::Matrix4f scale = MATH::Matrix4f::Scaling(keyframe.scale);

		MATH::Matrix4f localTransform = translation * rotation * scale;

		return localTransform;
	}

	AnimGlobalTransform::AnimGlobalTransform(Skeleton* skeleton) : mSkeleton(skeleton) {
		for (int i = 0; i < MAXBONES; i++)
			mTransforms.transforms[i] = MATH::Matrix4f(1.0f);
	}

	AnimGlobalTransform::~AnimGlobalTransform() {

	}

	PoseTransforms* AnimGlobalTransform::generateTransforms(PoseTransforms* localTransforms) {
		auto& joints = mSkeleton->joints();

		for (uint32_t i = 0; i < mSkeleton->getNumJolts(); i++) {
			if (joints[i].parentIndex == -1) {
				mTransforms.transforms[i] = localTransforms->transforms[i];
			}
			else {
				mTransforms.transforms[i] = mTransforms.transforms[joints[i].parentIndex] * localTransforms->transforms[i];
			}
		}

		return &mTransforms;
	}
}

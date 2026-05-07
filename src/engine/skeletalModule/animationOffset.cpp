#include "animationOffset.h"

namespace IKIGAI::SKELETON {
	AnimOffset::AnimOffset(Skeleton* skeleton) : mSkeleton(skeleton) {

	}

	AnimOffset::~AnimOffset() = default;

	PoseTransforms* AnimOffset::offset(PoseTransforms* transforms) {
		auto& joints = mSkeleton->joints();
		for (uint32_t i = 0; i < mSkeleton->getNumJolts(); i++) {
			mTransforms.transforms[i] = (transforms->transforms[i] * joints[i].offsetTransform);
		}
		return &mTransforms;
	}
}

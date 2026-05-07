#pragma once

#include "skeleton.h"

namespace IKIGAI::SKELETON {
	class AnimLookAtIK {
	public:
		AnimLookAtIK(Skeleton* skeleton);
		~AnimLookAtIK();

		PoseTransforms* lookAt(PoseTransforms* input, PoseTransforms* inputLocal, const MATH::Vector3f& target, float maxAngle, const std::string& joint);

	private:
		Skeleton* mSkeleton;
		PoseTransforms mTransforms;
	};
}

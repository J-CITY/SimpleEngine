#pragma once

#include "skeleton.h"

namespace IKIGAI::SKELETON {
	class AnimOffset {
	public:
		AnimOffset(Skeleton* skeleton);
		~AnimOffset();
		PoseTransforms* offset(PoseTransforms* transforms);

	private:
		Skeleton* mSkeleton;
		PoseTransforms mTransforms;
	};
}

#include "skeleton.h"
namespace IKIGAI::SKELETON {
	//TODO: skeletal, animation component
	Skeleton::Skeleton() {
		mNumJoints = 0;
	}

	Skeleton::~Skeleton() = default;


	int32_t Skeleton::findJointIndex(const std::string& channelName) const {
		for (int i = 0; i < mJoints.size(); i++) {
			if (mJoints[i].name == channelName) {
				return i;
			}
		}
		return -1;
	}
}

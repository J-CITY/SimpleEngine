#pragma once

#include "animation.h"
#include <unordered_set>
#include "mathModule/math.h"

namespace IKIGAI::SKELETON {
	struct Joint {
		std::string name;
		MATH::Matrix4f offsetTransform;
		int32_t parentIndex;
	};

	class Skeleton {
	public:
		Skeleton();
		~Skeleton();
		int32_t findJointIndex(const std::string& channelName) const;

		int32_t getNumJolts() const { return mNumJoints; }
		const std::vector<Joint>& joints() const { return mJoints; }
		std::vector<Joint>& joints() { return mJoints; }  // non-const for AssimpParser

		void setNumJoints(uint32_t num) { mNumJoints = num; }

	private:
		uint32_t mNumJoints;
		std::vector<Joint> mJoints;
	};
}

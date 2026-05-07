#include "animationLookatIK.h"
#include <algorithm>

namespace IKIGAI::SKELETON {
	AnimLookAtIK::AnimLookAtIK(Skeleton* skeleton) : mSkeleton(skeleton) {

	}

	AnimLookAtIK::~AnimLookAtIK() = default;

	PoseTransforms* AnimLookAtIK::lookAt(PoseTransforms* input, PoseTransforms* inputLocal, const MATH::Vector3f& target, float maxAngle, const std::string& joint) {
		int32_t idx = mSkeleton->findJointIndex(joint);

		if (idx != -1) {
			MATH::Matrix4f boneMat = input->transforms[idx];
			boneMat(0, 3) = 0.0f;
			boneMat(1, 3) = 0.0f;
			boneMat(2, 3) = 0.0f;
			boneMat(3, 3) = 1.0f;

			MATH::Matrix4f toBoneSpace = MATH::Matrix4f::Inverse(boneMat);

			MATH::Vector4f boneFwd = toBoneSpace * MATH::Vector4f(0.0f, 0.0f, 1.0f, 1.0f);
			MATH::Vector3f boneFwdDir = MATH::Vector3f::Normalize(MATH::Vector3f(boneFwd.x, boneFwd.y, boneFwd.z));

			MATH::Matrix4f globalToLocal = MATH::Matrix4f::Inverse(input->transforms[idx]);

			MATH::Vector4f localTarget = toBoneSpace * MATH::Vector4f(target, 1.0f);
			MATH::Vector3f localTargetDir = MATH::Vector3f::Normalize(MATH::Vector3f(localTarget.x, localTarget.y, localTarget.z));

			MATH::Vector3f rotationAxis = MATH::Vector3f::Cross(boneFwdDir, localTargetDir);
			rotationAxis = MATH::Vector3f::Normalize(rotationAxis);

			float angle = acosf(MATH::Vector3f::Dot(localTargetDir, boneFwdDir));

			
			angle = std::min(angle, MATH::TO_RADIANS(maxAngle));

			MATH::Matrix4f rotationMat = MATH::Matrix4f::Rotation(angle, rotationAxis);
			input->transforms[idx] = input->transforms[idx] * rotationMat;

			auto& joints = mSkeleton->joints();

			for (uint32_t i = (idx + 1); i < mSkeleton->getNumJolts(); i++) {
				if (i > idx && joints[i].parentIndex < idx) {
					break;
				}
				if (joints[i].parentIndex >= idx) {
					if (joints[i].parentIndex == -1) {
						input->transforms[i] = inputLocal->transforms[i];
					} else {
						input->transforms[i] = input->transforms[joints[i].parentIndex] * inputLocal->transforms[i];
					}
				}
			}

			return input;
		}

		return nullptr;
	}
}

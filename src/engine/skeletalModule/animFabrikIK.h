#pragma once

#include "skeleton.h"

#define MAXIKCHAINSIZE 8

namespace IKIGAI::SKELETON {
	class AnimFabrikIK {
	public:
		AnimFabrikIK(Skeleton* skeleton);
		~AnimFabrikIK();

		PoseTransforms* solve(MATH::Matrix4f model, PoseTransforms* localTransforms, PoseTransforms* globalTransforms, const MATH::Vector3f& endEffector, const std::string& startBone, const std::string& endBone);
		uint32_t getNumIterations() const { return mIterations; }
		void setIterations(uint32_t itr) { mIterations = itr; }

	private:
		int32_t findSourceChainData(MATH::Matrix4f model, int32_t startIdx, int32_t endIdx, PoseTransforms* globalTransforms);
		void forwardIk(int32_t endIdx, const MATH::Vector3f& endEffector);
		void backwardIk(int32_t endIdx, const MATH::Vector3f& endEffector);
		void modifyTransforms(MATH::Matrix4f model, int32_t startIdx, int32_t endIdx, PoseTransforms* localTransforms);

	private:
		float mBoneLengths[MAXIKCHAINSIZE - 1];
		MATH::Vector3f mSourceJointPos[MAXIKCHAINSIZE];
		MATH::Vector3f mIterationJointPos[MAXIKCHAINSIZE];

		uint32_t mIterations = 16;
		Skeleton* mSkeleton;
		PoseTransforms mTransforms;
	};
}

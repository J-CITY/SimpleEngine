#pragma once

#include "skeleton.h"

#include "iAnimationPlayable.h"

namespace IKIGAI::SKELETON {
	class AnimSample : public IAnimationPlayable {
	public:
		AnimSample(Skeleton* skeleton, Animation* animation);
		~AnimSample() override;
		void update(float dt) override;
		Pose* getPose() override;
		std::unique_ptr<IAnimationPlayable> clone() const override;
		
		void setPlaybackRate(float rate);
		float playbackRate();

	private:
		MATH::Vector3f interpolateTranslation(const MATH::Vector3f& a, const MATH::Vector3f& b, float t);
		MATH::Vector3f interpolateScale(const MATH::Vector3f& a, const MATH::Vector3f& b, float t);
		MATH::QuaternionF interpolateRotation(const MATH::QuaternionF& a, const MATH::QuaternionF& b, float t);
		uint32_t findTranslationKey(const std::vector<TranslationKey>& translations, double ticks);
		uint32_t findRotationKey(const std::vector<RotationKey>& rotations, double ticks);
		uint32_t findScaleKey(const std::vector<ScaleKey>& scale, double ticks);

	private:
		double mGlobalTime;
		double mLocalTime;
		float mLocalTimeNormalized;
		Skeleton* mSkeleton;
		Animation* mAnimation;
		float mPlaybackRate;
		Pose mPose;
	};
}

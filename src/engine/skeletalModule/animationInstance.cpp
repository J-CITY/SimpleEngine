#include "animationInstance.h"
#include <cmath>
#include <algorithm>
#ifdef __EMSCRIPTEN__
#include <iostream>
#endif

namespace IKIGAI::SKELETON {
AnimSample::AnimSample(std::shared_ptr<Skeleton> skeleton, std::shared_ptr<Animation> animation)
	: mSkeleton(skeleton)
	, mAnimation(animation)
	, mPlaybackRate(1.0f)
	, mGlobalTime(0.0) { }

AnimSample::~AnimSample() = default;

void AnimSample::update(float dt) {
	if (!mSkeleton || !mAnimation) {
		return;
	}

	mGlobalTime += (dt * mPlaybackRate); // dt is Delta Time in seconds.

	float ticksPerSecond = (float)(mAnimation->ticksPerSecond != 0.0 ? mAnimation->ticksPerSecond : 25.0f);
	double durationInTicks = mAnimation->durationInTicks;
	if (durationInTicks <= 0.0) {
		durationInTicks = 1.0;
	}
	float timeInTicks = ticksPerSecond * static_cast<float>(mGlobalTime);
	mLocalTime = mLooping ? fmod(static_cast<double>(timeInTicks), durationInTicks)
		: std::clamp(static_cast<double>(timeInTicks), 0.0, durationInTicks);
	if (mLocalTime < 0.0) {
		mLocalTime += durationInTicks;
	}
	mLocalTimeNormalized = static_cast<float>(mLocalTime / durationInTicks);
#ifdef __EMSCRIPTEN__
	static int poseLogs = 0;
	if (poseLogs < 8) {
		++poseLogs;
		std::cout << "[web] AnimSample dt=" << dt << " global=" << mGlobalTime << " localTicks=" << mLocalTime
				  << " durationTicks=" << mAnimation->durationInTicks << " tps=" << mAnimation->ticksPerSecond
				  << " channels=" << mAnimation->channels.size() << " joints=" << mSkeleton->getNumJolts() << std::endl;
		std::cout.flush();
	}
#endif

	mPose.numKeyframes = mSkeleton->getNumJolts();
	const uint32_t channelCount = static_cast<uint32_t>(mAnimation->channels.size());

	for (int i = 0; i < mSkeleton->getNumJolts(); i++) {
		Keyframe result = i < mSkeleton->joints().size() ? mSkeleton->joints()[i].bindPose : Keyframe{};
		if (static_cast<uint32_t>(i) >= channelCount) {
			mPose.keyframes[i] = result;
			continue;
		}
		const AnimationChannel& channel = mAnimation->channels[i];

		// Calculate interpolated translation
		{
			if (channel.translationKeyframes.size() == 0) {
				// Unanimated channels retain the skeleton's local bind transform.
			}
			else {
				const uint32_t idx1 = findTranslationKey(channel.translationKeyframes, mLocalTime);
				const uint32_t idx2 = idx1 + 1;

				if (channel.translationKeyframes.size() == 1) {
					result.translation = channel.translationKeyframes[idx1].translation;
				}
				else {
					float delta = (float)(channel.translationKeyframes[idx2].time - channel.translationKeyframes[idx1].time);
					float factor = delta > 0 ? std::clamp(float((mLocalTime - channel.translationKeyframes[idx1].time) / delta), 0.f, 1.f) : 0.f;

					result.translation = interpolateTranslation(
						channel.translationKeyframes[idx1].translation, channel.translationKeyframes[idx2].translation, factor);
				}
			}
		}

		// Calculate interpolated rotation
		{
			if (channel.rotationKeyframes.size() == 0) {
			}
			else {
				const uint32_t idx1 = findRotationKey(channel.rotationKeyframes, mLocalTime);
				const uint32_t idx2 = idx1 + 1;

				if (channel.rotationKeyframes.size() == 1) {
					result.rotation = channel.rotationKeyframes[idx1].rotation;
				}
				else {
					float delta = (float)(channel.rotationKeyframes[idx2].time - channel.rotationKeyframes[idx1].time);
					float factor = delta > 0 ? std::clamp(float((mLocalTime - channel.rotationKeyframes[idx1].time) / delta), 0.f, 1.f) : 0.f;

					result.rotation = interpolateRotation(channel.rotationKeyframes[idx1].rotation, channel.rotationKeyframes[idx2].rotation, factor);
				}
			}
		}

		// Calculate interpolated scale
		{
			if (channel.scaleKeyframes.size() == 0) {
			}
			else {
				const uint32_t idx1 = findScaleKey(channel.scaleKeyframes, mLocalTime);
				const uint32_t idx2 = idx1 + 1;

				if (channel.scaleKeyframes.size() == 1) {
					result.scale = channel.scaleKeyframes[idx1].scale;
				}
				else {
					float delta = (float)(channel.scaleKeyframes[idx2].time - channel.scaleKeyframes[idx1].time);
					float factor = delta > 0 ? std::clamp(float((mLocalTime - channel.scaleKeyframes[idx1].time) / delta), 0.f, 1.f) : 0.f;

					result.scale = interpolateScale(channel.scaleKeyframes[idx1].scale, channel.scaleKeyframes[idx2].scale, factor);
				}
			}
		}

		mPose.keyframes[i] = result;
	}
}

Pose* AnimSample::getPose() {
	return &mPose;
}

void AnimSample::setPlaybackRate(float rate) {
	if (!std::isfinite(rate) || rate < 0.0f || rate > 8.0f) {
		return;
	}

	mPlaybackRate = rate;
}

float AnimSample::playbackRate() {
	return mPlaybackRate;
}

float AnimSample::duration() const {
	if (!mAnimation) return 0;
	return static_cast<float>(mAnimation->durationInTicks / (mAnimation->ticksPerSecond > 0 ? mAnimation->ticksPerSecond : 25.0));
}
void AnimSample::seek(float seconds) {
	if (!std::isfinite(seconds)) return;
	mGlobalTime = std::max(0.f, seconds);
	update(0);
}

MATH::Vector3f AnimSample::interpolateTranslation(const MATH::Vector3f& a, const MATH::Vector3f& b, float t) {
	return MATH::Vector3f::Lerp(a, b, t);
}

MATH::Vector3f AnimSample::interpolateScale(const MATH::Vector3f& a, const MATH::Vector3f& b, float t) {
	return MATH::Vector3f::Lerp(a, b, t);
}

MATH::QuaternionF AnimSample::interpolateRotation(const MATH::QuaternionF& a, const MATH::QuaternionF& b, float t) {
	return MATH::QuaternionF::Slerp(a, b, t);
}

uint32_t AnimSample::findTranslationKey(const std::vector<TranslationKey>& translations, double ticks) {
	uint32_t idx = translations.size() > 1 ? static_cast<uint32_t>(translations.size() - 2) : 0;

	for (uint32_t i = 0; i < (translations.size() - 1); i++) {
		if (ticks < translations[i + 1].time) {
			idx = i;
			break;
		}
	}

	return idx;
}

uint32_t AnimSample::findRotationKey(const std::vector<RotationKey>& rotations, double ticks) {
	uint32_t idx = rotations.size() > 1 ? static_cast<uint32_t>(rotations.size() - 2) : 0;

	for (uint32_t i = 0; i < (rotations.size() - 1); i++) {
		if (ticks < rotations[i + 1].time) {
			idx = i;
			break;
		}
	}

	return idx;
}

uint32_t AnimSample::findScaleKey(const std::vector<ScaleKey>& scale, double ticks) {
	uint32_t idx = scale.size() > 1 ? static_cast<uint32_t>(scale.size() - 2) : 0;

	for (uint32_t i = 0; i < (scale.size() - 1); i++) {
		if (ticks < scale[i + 1].time) {
			idx = i;
			break;
		}
	}

	return idx;
}

std::unique_ptr<IAnimationPlayable> AnimSample::clone() const {
	auto result = std::make_unique<AnimSample>(mSkeleton, mAnimation);
	result->setLooping(mLooping);
	result->setPlaybackRate(mPlaybackRate);
	result->seek(static_cast<float>(mGlobalTime));
	return result;
}
}

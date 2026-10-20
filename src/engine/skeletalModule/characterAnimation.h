#pragma once
#include "skeleton.h"
#include <algorithm>
#include <array>
#include <cmath>

namespace IKIGAI::SKELETON::CHARACTER {
struct Rig {
	std::string root = "Hips";
	std::vector<std::string> feet = { "LeftFoot", "RightFoot", "LeftToeBase", "RightToeBase" };
	std::vector<std::string> hands = { "LeftHand", "RightHand" };
	float unitsToMeters = .01f, soleOffset = .03f, rootHeight = .9f;
};
inline float smooth(float a, float b, float t) {
	const float x = std::clamp((t - a) / std::max(.0001f, b - a), 0.f, 1.f);
	return x * x * (3 - 2 * x);
}
inline float angleDelta(float from, float to) {
	return std::remainder(to - from, 360.f);
}
inline float strideOffset(const std::array<float, 32>& reference, const std::array<float, 32>& candidate) {
	float best = -1e20f;
	int offset = 0;
	for (int shift = 0; shift < 32; ++shift) {
		float score = 0;
		for (int i = 0; i < 32; ++i) {
			score += reference[i] * candidate[(i + shift) % 32];
		}
		if (score > best) {
			best = score;
			offset = shift;
		}
	}
	return float(offset) / 32;
}
// In model space, before inverse bind matrices. Keep the same TRS order as skinning.
inline std::array<MATH::Vector3f, MAXBONES> positions(const Skeleton& skeleton, const Pose& pose) {
	std::array<MATH::Matrix4f, MAXBONES> transforms;
	std::array<MATH::Vector3f, MAXBONES> result {};
	for (unsigned i = 0; i < pose.numKeyframes; ++i) {
		const auto& k = pose.keyframes[i];
		auto local = MATH::Matrix4f::Translation(k.translation) * MATH::QuaternionF::ToMatrix4(k.rotation) * MATH::Matrix4f::Scaling(k.scale);
		const auto parent = skeleton.joints()[i].parentIndex;
		transforms[i] = parent < 0 ? local : transforms[parent] * local;
		result[i] = transforms[i] * MATH::Vector3f {};
	}
	return result;
}
inline MATH::Vector3f feet(const Skeleton& skeleton, const Pose& pose, const Rig& rig = {}) {
	const auto points = positions(skeleton, pose);
	MATH::Vector3f result {};
	float lowest = 1e10f;
	int count = 0;
	for (const auto& name : rig.feet) {
		const int index = skeleton.findJointIndex(name);
		if (index < 0) {
			continue;
		}
		const auto p = points[index] * rig.unitsToMeters;
		result += p;
		++count;
		lowest = std::min(lowest, p.y);
	}
	if (count) {
		result = result / float(count);
		result.y = lowest - rig.soleOffset;
	}
	return result;
}
inline MATH::Vector3f hand(const Skeleton& skeleton, const Pose& pose, bool single = false, const Rig& rig = {}) {
	const auto points = positions(skeleton, pose);
	MATH::Vector3f sum {};
	int count = 0;
	for (const auto& name : rig.hands) {
		const int i = skeleton.findJointIndex(name);
		if (i < 0) {
			continue;
		}
		sum += points[i];
		++count;
		if (single) {
			break;
		}
	}
	return count ? sum * (rig.unitsToMeters / float(count)) : sum;
}
// Blend an authored trajectory into an obstacle contact. Contact is exact during
// the planted interval, then the endpoint correction takes over smoothly.
inline MATH::Vector3f anchorOrigin(
	MATH::Vector3f startOrigin, MATH::Vector3f endOrigin, MATH::Vector3f contact, MATH::Vector3f authoredHand, float t, float plant, float release) {
	const auto base = MATH::Vector3f::Lerp(startOrigin, endOrigin, smooth(0, 1, t));
	const float weight = smooth(std::max(0.f, plant - .12f), plant, t) * (1 - smooth(release, std::min(1.f, release + .18f), t));
	return MATH::Vector3f::Lerp(base, contact - authoredHand, weight);
}
}

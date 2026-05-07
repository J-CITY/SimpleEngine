#include "animation.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include "skeleton.h"

namespace IKIGAI::SKELETON {
	MATH::Vector3f translationDelta(const MATH::Vector3f& reference, MATH::Vector3f additive) {
		return additive - reference;
	}

	MATH::Vector3f scaleDelta(const MATH::Vector3f& reference, MATH::Vector3f additive) {
		return additive / reference;
	}

	MATH::QuaternionF rotationDelta(const MATH::QuaternionF& reference, MATH::QuaternionF additive) {
		return MATH::QuaternionF::Conjugate(reference) * additive;
	}

	Animation* Animation::load(const std::string& name, Skeleton* skeleton, bool additive, Animation* additiveReference) {
		const aiScene* scene;
		Assimp::Importer importer;
		scene = importer.ReadFile(name, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs);

		if (!scene) {
			//LOG_ERROR("Failed to load animation file : " + name);
			return nullptr;
		}

		if (!scene->mAnimations) {
			//LOG_ERROR("No Animations available in file : " + name);
			return nullptr;
		}

		aiAnimation* animation = scene->mAnimations[0];

		Animation* outputAnimation = new Animation();

		outputAnimation->channels.resize(skeleton->getNumJolts());
		outputAnimation->name = std::string(animation->mName.C_Str());
		outputAnimation->duration = animation->mDuration / animation->mTicksPerSecond;
		outputAnimation->durationInTicks = animation->mDuration;
		outputAnimation->ticksPerSecond = animation->mTicksPerSecond;
		outputAnimation->keyframeCount = animation->mChannels[0]->mNumPositionKeys;

		for (int i = 0; i < scene->mAnimations[0]->mNumChannels; i++) {
			aiNodeAnim* channel = scene->mAnimations[0]->mChannels[i];
			std::string channelName = trimmedName(channel->mNodeName.C_Str());

			int jointIndex = skeleton->findJointIndex(channelName);

			if (jointIndex != -1) {
				outputAnimation->channels[jointIndex].jointName = channelName;

				// Translation Keyframes
				outputAnimation->channels[jointIndex].translationKeyframes.resize(channel->mNumPositionKeys);

				MATH::Vector3f referenceTranslation;

				if (channel->mNumPositionKeys > 0) {
					if (additiveReference) {
						AnimationChannel& additiveChannel = additiveReference->channels[jointIndex];

						if (additiveChannel.translationKeyframes.size() > 0)
							referenceTranslation = additiveChannel.translationKeyframes[0].translation;
					} else
						referenceTranslation = MATH::Vector3f(channel->mPositionKeys[0].mValue.x, channel->mPositionKeys[0].mValue.y, channel->mPositionKeys[0].mValue.z);
				}


				for (int j = 0; j < channel->mNumPositionKeys; j++) {
					outputAnimation->channels[jointIndex].translationKeyframes[j].time = channel->mPositionKeys[j].mTime;

					outputAnimation->channels[jointIndex].translationKeyframes[j].translation = MATH::Vector3f(channel->mPositionKeys[j].mValue.x,
						channel->mPositionKeys[j].mValue.y,
						channel->mPositionKeys[j].mValue.z);

					if (additive)
						outputAnimation->channels[jointIndex].translationKeyframes[j].translation = translationDelta(referenceTranslation, outputAnimation->channels[jointIndex].translationKeyframes[j].translation);
				}

				// Rotation Keyframes
				outputAnimation->channels[jointIndex].rotationKeyframes.resize(channel->mNumRotationKeys);

				MATH::QuaternionF referenceRotation;

				if (channel->mNumRotationKeys > 0) {
					if (additiveReference) {
						AnimationChannel& additiveChannel = additiveReference->channels[jointIndex];

						if (additiveChannel.rotationKeyframes.size() > 0)
							referenceRotation = additiveChannel.rotationKeyframes[0].rotation;
					} else {
						referenceRotation = MATH::QuaternionF(channel->mRotationKeys[0].mValue.w,
							channel->mRotationKeys[0].mValue.x,
							channel->mRotationKeys[0].mValue.y,
							channel->mRotationKeys[0].mValue.z);
					}
				}

				for (int j = 0; j < channel->mNumRotationKeys; j++) {
					outputAnimation->channels[jointIndex].rotationKeyframes[j].time = channel->mRotationKeys[j].mTime;

					outputAnimation->channels[jointIndex].rotationKeyframes[j].rotation = MATH::QuaternionF(channel->mRotationKeys[j].mValue.w,
						channel->mRotationKeys[j].mValue.x,
						channel->mRotationKeys[j].mValue.y,
						channel->mRotationKeys[j].mValue.z);

					if (additive)
						outputAnimation->channels[jointIndex].rotationKeyframes[j].rotation = rotationDelta(referenceRotation, outputAnimation->channels[jointIndex].rotationKeyframes[j].rotation);
				}

				// Scale Keyframes
				outputAnimation->channels[jointIndex].scaleKeyframes.resize(channel->mNumScalingKeys);

				MATH::Vector3f referenceScale;

				if (channel->mNumScalingKeys > 0) {
					if (additiveReference) {
						AnimationChannel& additiveChannel = additiveReference->channels[jointIndex];

						if (additiveChannel.scaleKeyframes.size() > 0)
							referenceScale = additiveChannel.scaleKeyframes[0].scale;
					} else
						referenceScale = MATH::Vector3f(channel->mScalingKeys[0].mValue.x, channel->mScalingKeys[0].mValue.y, channel->mScalingKeys[0].mValue.z);
				}

				for (int j = 0; j < channel->mNumScalingKeys; j++) {
					outputAnimation->channels[jointIndex].scaleKeyframes[j].time = channel->mScalingKeys[j].mTime;

					outputAnimation->channels[jointIndex].scaleKeyframes[j].scale = MATH::Vector3f(channel->mScalingKeys[j].mValue.x,
						channel->mScalingKeys[j].mValue.y,
						channel->mScalingKeys[j].mValue.z);

					if (additive)
						outputAnimation->channels[jointIndex].scaleKeyframes[j].scale = scaleDelta(referenceScale, outputAnimation->channels[jointIndex].scaleKeyframes[j].scale);
				}
			}
		}

		return outputAnimation;
	}

	std::string trimmedName(const std::string& name) {
		size_t pos = name.find_first_of(':');
		if (pos != std::string::npos) {
			return name.substr(pos + 1);
		}
		return name;
	}
}

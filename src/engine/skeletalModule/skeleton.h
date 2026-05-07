#pragma once

#include "animation.h"
#include <unordered_set>
#include "mathModule/math.h"

struct aiNode;
struct aiBone;
struct aiScene;

namespace IKIGAI::SKELETON {
	struct Joint {
		std::string name;
		MATH::Matrix4f offsetTransform;
		int32_t parentIndex;
	};

	class Skeleton {
	public:
		static Skeleton* create(const aiScene* scene);

		Skeleton();
		~Skeleton();
		int32_t findJointIndex(const std::string& channelName) const;

		int32_t getNumJolts() const { return mNumJoints; }
		const std::vector<Joint>& joints() const { return mJoints; }

	private:
		void buildBoneList(aiNode* node, const aiScene* scene, std::vector<aiBone*>& tempBoneList, std::unordered_set<std::string>& boneMap);
		void buildSkeleton(aiNode* node, int boneIndex, const aiScene* scene, std::vector<aiBone*>& tempBoneList);

		uint32_t mNumJoints;
		std::vector<Joint> mJoints;
	};
}

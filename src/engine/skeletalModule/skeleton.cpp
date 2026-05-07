#include "skeleton.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <iostream>

//TODO: add loader in resource system
//TODO: add support in pack loader
//TODO: add components
//TODO: resource file for skeleton and blend animation etc

namespace {
	void printSceneHeirarchy(aiNode* node) {
		if (node) {
			std::cout << node->mName.C_Str() << std::endl;

			for (int i = 0; i < node->mNumChildren; i++) {
				printSceneHeirarchy(node->mChildren[i]);
			}
		}
	}
}
namespace IKIGAI::SKELETON {
	Skeleton* Skeleton::create(const aiScene* scene) {
		Skeleton* skeleton = new Skeleton();

		std::vector<aiBone*> tempBoneList(256);
		std::unordered_set<std::string> boneMap;

		std::cout << "\nBegin Print Scene\n" << std::endl;
		printSceneHeirarchy(scene->mRootNode);
		std::cout << "\nEnd Print Scene\n" << std::endl;

		skeleton->buildBoneList(scene->mRootNode, scene, tempBoneList, boneMap);
		skeleton->mJoints.reserve(skeleton->mNumJoints);
		skeleton->buildSkeleton(scene->mRootNode, 0, scene, tempBoneList);

		std::cout << "\nBegin Print Joint List\n" << std::endl;

		for (int i = 0; i < skeleton->mJoints.size(); i++) {
			auto& joint = skeleton->mJoints[i];
			std::cout << "Index: " << i << ", Name: " << joint.name << ", Parent: " << joint.parentIndex << std::endl;
		}

		std::cout << "\nEnd Print Joint List\n" << std::endl;

		return skeleton;
	}

	Skeleton::Skeleton() {
		mNumJoints = 0;
	}

	Skeleton::~Skeleton() = default;

	void Skeleton::buildBoneList(aiNode* node, const aiScene* scene, std::vector<aiBone*>& tempBoneList, std::unordered_set<std::string>& boneMap) {
		for (int i = 0; i < node->mNumMeshes; i++) {
			aiMesh* currentMesh = scene->mMeshes[node->mMeshes[i]];

			for (int j = 0; j < currentMesh->mNumBones; j++) {
				std::string boneName = std::string(currentMesh->mBones[j]->mName.C_Str());
				if (boneMap.find(boneName) == boneMap.end()) {
					tempBoneList[mNumJoints] = currentMesh->mBones[j];
					mNumJoints++;
					boneMap.insert(boneName);
				}
			}
		}

		for (int i = 0; i < node->mNumChildren; i++) {
			buildBoneList(node->mChildren[i], scene, tempBoneList, boneMap);
		}
	}

	void Skeleton::buildSkeleton(aiNode* node, int boneIndex, const aiScene* scene, std::vector<aiBone*>& tempBoneList) {
		std::string nodeName = trimmedName(node->mName.C_Str());

		for (int i = 0; i < mNumJoints; i++) {
			std::string boneName = trimmedName(tempBoneList[i]->mName.C_Str());

			if (boneName == nodeName) {
				Joint joint;

				joint.name = boneName;
				auto& m = tempBoneList[i]->mOffsetMatrix;
				joint.offsetTransform = MATH::Matrix4f(
					m.a1, m.a2, m.a3, m.a4,
					m.b1, m.b2, m.b3, m.b4,
					m.c1, m.c2, m.c3, m.c4,
					m.d1, m.d2, m.d3, m.d4
				);

				aiNode* parent = node->mParent;
				int index;

				while (parent) {
					index = findJointIndex(trimmedName(parent->mName.C_Str()));

					if (index == -1) {
						parent = parent->mParent;
					} else {
						break;
					}
				}

				joint.parentIndex = index;
				mJoints.push_back(joint);
				break;
			}
		}

		for (int i = 0; i < node->mNumChildren; i++) {
			buildSkeleton(node->mChildren[i], mNumJoints, scene, tempBoneList);
		}
	}

	int32_t Skeleton::findJointIndex(const std::string& channelName) const {
		for (int i = 0; i < mJoints.size(); i++) {
			if (mJoints[i].name == channelName) {
				return i;
			}
		}
		return -1;
	}
}

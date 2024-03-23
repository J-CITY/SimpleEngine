#include "bone.h"

#include <cassert>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include "../parser/assimpParser.h"

using namespace IKIGAI;
using namespace IKIGAI::RESOURCES;
#define MAX_BONES 100

Bone::Bone(const std::string& name, int ID, const aiNodeAnim* channel):
	m_Name(name),
	m_ID(ID),
	m_LocalTransform(1.0f) {
	m_NumPositions = channel->mNumPositionKeys;

	for (int positionIndex = 0; positionIndex < m_NumPositions; ++positionIndex) {
		aiVector3D aiPosition = channel->mPositionKeys[positionIndex].mValue;
		float timeStamp = channel->mPositionKeys[positionIndex].mTime;
		KeyPosition data;
		data.position = ConvertVector3(aiPosition);
		data.timeStamp = timeStamp;
		m_Positions.push_back(data);
	}

	m_NumRotations = channel->mNumRotationKeys;
	for (int rotationIndex = 0; rotationIndex < m_NumRotations; ++rotationIndex) {
		aiQuaternion aiOrientation = channel->mRotationKeys[rotationIndex].mValue;
		float timeStamp = channel->mRotationKeys[rotationIndex].mTime;
		KeyRotation data;
		data.orientation = ConvertQuaternion(aiOrientation);
		data.timeStamp = timeStamp;
		m_Rotations.push_back(data);
	}

	m_NumScalings = channel->mNumScalingKeys;
	for (int keyIndex = 0; keyIndex < m_NumScalings; ++keyIndex) {
		aiVector3D scale = channel->mScalingKeys[keyIndex].mValue;
		float timeStamp = channel->mScalingKeys[keyIndex].mTime;
		KeyScale data;
		data.scale = ConvertVector3(scale);
		data.timeStamp = timeStamp;
		m_Scales.push_back(data);
	}
}

void Bone::Update(float animationTime) {
	auto translation = InterpolatePosition(animationTime);
	auto rotation = InterpolateRotation(animationTime);
	auto scale = InterpolateScaling(animationTime);
	m_LocalTransform = translation * (rotation * scale);
}

std::string Bone::GetBoneName() const
{ return m_Name; }

int Bone::GetBoneID()
{ return m_ID; }

int Bone::GetPositionIndex(float animationTime) {
	for (int index = 0; index < m_NumPositions - 1; ++index) {
		if (animationTime < m_Positions[index + 1].timeStamp)
			return index;
	}
	assert(0);
	return 0;
}

int Bone::GetRotationIndex(float animationTime) {
	for (int index = 0; index < m_NumRotations - 1; ++index) {
		if (animationTime < m_Rotations[index + 1].timeStamp)
			return index;
	}
	assert(0);
	return 0;
}

int Bone::GetScaleIndex(float animationTime) {
	for (int index = 0; index < m_NumScalings - 1; ++index) {
		if (animationTime < m_Scales[index + 1].timeStamp)
			return index;
	}
	assert(0);
	return 0;
}

float Bone::GetScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime) {
	float scaleFactor = 0.0f;
	float midWayLength = animationTime - lastTimeStamp;
	float framesDiff = nextTimeStamp - lastTimeStamp;
	scaleFactor = midWayLength / framesDiff;
	return scaleFactor;
}

MATH::Matrix4f Bone::InterpolatePosition(float animationTime) {
    if (1 == m_NumPositions) {
        return MATH::Matrix4f::Translation(m_Positions[0].position);
    }
    int p0Index = GetPositionIndex(animationTime);
    int p1Index = p0Index + 1;
    float scaleFactor = GetScaleFactor(m_Positions[p0Index].timeStamp,
        m_Positions[p1Index].timeStamp, animationTime);
    auto finalPosition = MATH::Vector3f::Mix(m_Positions[p0Index].position, m_Positions[p1Index].position, scaleFactor);
	return MATH::Matrix4f::Translation(finalPosition);
}

MATH::Matrix4f Bone::InterpolateRotation(float animationTime) {
    if (1 == m_NumRotations) {
        return MATH::QuaternionF::ToMatrix4(MATH::QuaternionF::Normalize(m_Rotations[0].orientation));
    }

    int p0Index = GetRotationIndex(animationTime);
    int p1Index = p0Index + 1;
    float scaleFactor = GetScaleFactor(m_Rotations[p0Index].timeStamp, m_Rotations[p1Index].timeStamp, animationTime);
    auto res = MATH::QuaternionF::Slerp(m_Rotations[p0Index].orientation, m_Rotations[p1Index].orientation, scaleFactor);
    return MATH::QuaternionF::ToMatrix4(MATH::QuaternionF::Normalize(res));
}

MATH::Matrix4f Bone::Bone::InterpolateScaling(float animationTime) {
    if (1 == m_NumScalings)
        return MATH::Matrix4f::Scaling(m_Scales[0].scale);

    int p0Index = GetScaleIndex(animationTime);
    int p1Index = p0Index + 1;
    float scaleFactor = GetScaleFactor(m_Scales[p0Index].timeStamp, m_Scales[p1Index].timeStamp, animationTime);
    auto finalScale = MATH::Vector3f::Mix(m_Scales[p0Index].scale, m_Scales[p1Index].scale, scaleFactor);
    return MATH::Matrix4f::Scaling(finalScale);
}

Animation::Animation() = default;

Animation::~Animation() = default;

Bone* Animation::FindBone(const std::string& name) {
	auto iter = std::ranges::find_if(m_Bones.begin(), m_Bones.end(), [&](const Bone& Bone) {
		return Bone.GetBoneName() == name;
	});
	if (iter == m_Bones.end()) return nullptr;
	else return &(*iter);
}

float Animation::GetTicksPerSecond()
{ return m_TicksPerSecond; }

float Animation::GetDuration()
{ return m_Duration; }

const AssimpNodeData& Animation::GetRootNode()
{ return m_RootNode; }

const std::map<std::string, RENDER::BoneInfo>& Animation::GetBoneIDMap() {
	return m_BoneInfoMap;
}

void Animation::ReadMissingBones(const aiAnimation& animation, RENDER::ModelInterface& model) {
	int size = animation.mNumChannels;

	auto& boneInfoMap = model.GetBoneInfoMap();//getting m_BoneInfoMap from Model class
	int& boneCount = model.GetBoneCount(); //getting the m_BoneCounter from Model class

	//reading channels(bones engaged in an animation and their keyframes)
	for (int i = 0; i < size; i++) {
		auto channel = animation.mChannels[i];
		std::string boneName = channel->mNodeName.data;

		if (boneInfoMap.find(boneName) == boneInfoMap.end()) {
			boneInfoMap[boneName].mId = boneCount;
			boneCount++;
		}
		m_Bones.push_back(Bone(channel->mNodeName.data,
		                       boneInfoMap[channel->mNodeName.data].mId, channel));
	}

	m_BoneInfoMap = boneInfoMap;
}

void Animation::ReadHeirarchyData(AssimpNodeData& dest, const aiNode* src) {
	assert(src);

	dest.name = src->mName.data;
	dest.transformation = ConvertMatrix4x4(src->mTransformation);
	dest.childrenCount = src->mNumChildren;

	for (int i = 0; i < src->mNumChildren; i++) {
		AssimpNodeData newData;
		ReadHeirarchyData(newData, src->mChildren[i]);
		dest.children.push_back(newData);
	}
}

Animation::Animation(const std::string& animationPath, RENDER::ModelInterface* model) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(animationPath, aiProcess_Triangulate);
    assert(scene && scene->mRootNode);
    auto animation = scene->mAnimations[0];
    m_Duration = animation->mDuration;
    m_TicksPerSecond = animation->mTicksPerSecond;
    aiMatrix4x4 globalTransformation = scene->mRootNode->mTransformation;
    globalTransformation = globalTransformation.Inverse();
    ReadHeirarchyData(m_RootNode, scene->mRootNode);
    ReadMissingBones(*animation, *model);
}


Animation::Animation(const aiAnimation& animation, const aiScene& scene, RENDER::ModelInterface& model) {
    m_Duration = animation.mDuration;
    m_TicksPerSecond = animation.mTicksPerSecond;
    aiMatrix4x4 globalTransformation = scene.mRootNode->mTransformation;
    globalTransformation = globalTransformation.Inverse();
    ReadHeirarchyData(m_RootNode, scene.mRootNode);
    ReadMissingBones(animation, model);
}

std::vector<MATH::Matrix4f> Animator::GetFinalBoneMatrices() {
	return m_FinalBoneMatrices;
}

std::map<std::string, std::shared_ptr<Animation>> Animation::LoadAnimations(const std::string& animationPath, RENDER::ModelInterface* model) {
    std::map<std::string, std::shared_ptr<Animation>> res;

    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(animationPath, aiProcess_Triangulate);
    assert(scene && scene->mRootNode);

    for (auto i = 0u; i < scene->mNumAnimations; i++) {
        auto animation = scene->mAnimations[i];
        auto name = animation->mName;
        res[name.C_Str()] = std::shared_ptr<Animation>(new Animation (*animation, *scene, *model));
    }
    return res;
}

Animator::Animator(Animation* animation) {
    m_CurrentTime = 0.0;
    m_CurrentAnimation = animation;

    m_FinalBoneMatrices.reserve(100);

    for (int i = 0; i < 100; i++)
        m_FinalBoneMatrices.push_back(MATH::Matrix4f(1.0f));
}

void Animator::UpdateAnimation(float dt) {
    if (!m_CurrentAnimation) {
        return;
    }
    m_DeltaTime = dt;
    if (m_CurrentAnimation) {
        m_CurrentTime += m_CurrentAnimation->GetTicksPerSecond() * dt;
        m_CurrentTime = fmod(m_CurrentTime, m_CurrentAnimation->GetDuration());
        CalculateBoneTransform(&m_CurrentAnimation->GetRootNode(), MATH::Matrix4f(1.0f));
    }
}

void Animator::PlayAnimation(Animation* pAnimation) {
    m_CurrentAnimation = pAnimation;
    m_CurrentTime = 0.0f;
}

void Animator::CalculateBoneTransform(const AssimpNodeData* node, MATH::Matrix4f parentTransform) {
    std::string nodeName = node->name;
    auto nodeTransform = node->transformation;

    Bone* Bone = m_CurrentAnimation->FindBone(nodeName);

    if (Bone) {
        Bone->Update(m_CurrentTime);
        nodeTransform = Bone->GetLocalTransform();
    }

    auto globalTransformation = parentTransform * nodeTransform;

    auto boneInfoMap = m_CurrentAnimation->GetBoneIDMap();
    if (boneInfoMap.find(nodeName) != boneInfoMap.end()) {
        int index = boneInfoMap[nodeName].mId;
        auto offset = boneInfoMap[nodeName].mOffset;
        m_FinalBoneMatrices[index] = globalTransformation * offset;
    }

    for (int i = 0; i < node->childrenCount; i++) {
        CalculateBoneTransform(&node->children[i], globalTransformation);
    }
}

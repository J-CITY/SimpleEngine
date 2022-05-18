#pragma once
#include <cassert>
#include <vector>

#include "../../render/Model.h"
#include "../../utils/math/Quaternion.h"
#include "../../utils/math/Vector3.h"
#include <glm/gtx/quaternion.hpp>
#include <glm/glm.hpp>

struct aiNodeAnim;

namespace KUMA {
	namespace RENDER {
        class Model;
	}
	namespace RESOURCES {
        /*struct KeyPosition {
	        MATHGL::Vector3 position;
            float timeStamp;
        };

        struct KeyRotation {
	        MATHGL::Quaternion orientation;
            float timeStamp;
        };

        struct KeyScale {
            MATHGL::Vector3 scale;
            float timeStamp;
        };

        class Bone {
        private:
            std::vector<KeyPosition> m_Positions;
            std::vector<KeyRotation> m_Rotations;
            std::vector<KeyScale> m_Scales;
            int m_NumPositions;
            int m_NumRotations;
            int m_NumScalings;

            MATHGL::Matrix4 m_LocalTransform;
            std::string m_Name;
            int m_ID;

        public:
            Bone(const std::string& name, int ID, const aiNodeAnim* channel);
            void Update(float animationTime);

            MATHGL::Matrix4 GetLocalTransform();
            std::string GetBoneName() const;
            int GetBoneID();
            int GetPositionIndex(float animationTime);
            int GetRotationIndex(float animationTime);
            int GetScaleIndex(float animationTime);
        private:
            float GetScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime);
            MATHGL::Matrix4 InterpolatePosition(float animationTime);
            MATHGL::Matrix4 InterpolateRotation(float animationTime);
            MATHGL::Matrix4 InterpolateScaling(float animationTime);
        };

        struct AssimpNodeData {
	        MATHGL::Matrix4 transformation;
            std::string name;
            int childrenCount;
            std::vector<AssimpNodeData> children;
        };*/

        /*class Animation {
        public:
            Animation() = default;

            Animation(const std::string& animationPath, RENDER::Model* model);

            ~Animation() = default;

            Bone* FindBone(const std::string& name);

            float GetTicksPerSecond();
            float GetDuration();
            const AssimpNodeData& GetRootNode();
            const std::map<std::string, RENDER::BoneInfo>& GetBoneIDMap();

        private:
            void ReadMissingBones(const aiAnimation* animation, RENDER::Model& model);
            void ReadHeirarchyData(AssimpNodeData& dest, const aiNode* src);
            float m_Duration;
            int m_TicksPerSecond;
            std::vector<Bone> m_Bones;
            AssimpNodeData m_RootNode;
            std::map<std::string, RENDER::BoneInfo> m_BoneInfoMap;
        };

        class Animator {
        public:
            Animator(Animation* Animation);
            void UpdateAnimation(float dt);
            void PlayAnimation(Animation* pAnimation);
            void CalculateBoneTransform(const AssimpNodeData* node, MATHGL::Matrix4 parentTransform);
            std::vector<MATHGL::Matrix4> GetFinalBoneMatrices();
        private:
            std::vector<MATHGL::Matrix4> m_FinalBoneMatrices;
            Animation* m_CurrentAnimation;
            float m_CurrentTime;
            float m_DeltaTime;
        };*/

        struct AssimpNodeData {
            MATHGL::Matrix4 transformation;
            std::string name;
            int childrenCount;
            std::vector<AssimpNodeData> children;
        };
        class AssimpGLMHelpers {
        public:

            static inline MATHGL::Matrix4 ConvertMatrixToGLMFormat(const aiMatrix4x4& from) {
                MATHGL::Matrix4 to;
                //the a,b,c,d in assimp is the row ; the 1,2,3,4 is the column
                to(0,0) = from.a1; to(1,0) = from.b1; to(2,0) = from.c1; to(3,0) = from.d1;
                to(0,1) = from.a2; to(1,1) = from.b2; to(2,1) = from.c2; to(3,1) = from.d2;
                to(0,2) = from.a3; to(1,2) = from.b3; to(2,2) = from.c3; to(3,2) = from.d3;
                to(0,3) = from.a4; to(1,3) = from.b4; to(2,3) = from.c4; to(3,3) = from.d4;
                return to;
            }

            static inline MATHGL::Vector3 GetGLMVec(const aiVector3D& vec) {
                return MATHGL::Vector3(vec.x, vec.y, vec.z);
            }

            static inline MATHGL::Quaternion GetGLMQuat(const aiQuaternion& pOrientation) {
                return MATHGL::Quaternion(pOrientation.x, pOrientation.y, pOrientation.z, pOrientation.w);
            }
        };

        struct KeyPosition {
            MATHGL::Vector3 position;
            float timeStamp;
        };

        struct KeyRotation {
            MATHGL::Quaternion orientation;
            float timeStamp;
        };

        struct KeyScale {
            MATHGL::Vector3 scale;
            float timeStamp;
        };

        class Bone {
        public:
            Bone(const std::string& name, int ID, const aiNodeAnim* channel)
                :
                m_Name(name),
                m_ID(ID),
                m_LocalTransform(1.0f) {
                m_NumPositions = channel->mNumPositionKeys;

                for (int positionIndex = 0; positionIndex < m_NumPositions; ++positionIndex) {
                    aiVector3D aiPosition = channel->mPositionKeys[positionIndex].mValue;
                    float timeStamp = channel->mPositionKeys[positionIndex].mTime;
                    KeyPosition data;
                    data.position = AssimpGLMHelpers::GetGLMVec(aiPosition);
                    data.timeStamp = timeStamp;
                    m_Positions.push_back(data);
                }

                m_NumRotations = channel->mNumRotationKeys;
                for (int rotationIndex = 0; rotationIndex < m_NumRotations; ++rotationIndex) {
                    aiQuaternion aiOrientation = channel->mRotationKeys[rotationIndex].mValue;
                    float timeStamp = channel->mRotationKeys[rotationIndex].mTime;
                    KeyRotation data;
                    data.orientation = AssimpGLMHelpers::GetGLMQuat(aiOrientation);
                    data.timeStamp = timeStamp;
                    m_Rotations.push_back(data);
                }

                m_NumScalings = channel->mNumScalingKeys;
                for (int keyIndex = 0; keyIndex < m_NumScalings; ++keyIndex) {
                    aiVector3D scale = channel->mScalingKeys[keyIndex].mValue;
                    float timeStamp = channel->mScalingKeys[keyIndex].mTime;
                    KeyScale data;
                    data.scale = AssimpGLMHelpers::GetGLMVec(scale);
                    data.timeStamp = timeStamp;
                    m_Scales.push_back(data);
                }
            }

            void Update(float animationTime) {
                auto translation = InterpolatePosition(animationTime);
                auto rotation = InterpolateRotation(animationTime);
                auto scale = InterpolateScaling(animationTime);
                m_LocalTransform = translation * (rotation * scale);
            }
            MATHGL::Matrix4 GetLocalTransform() { return m_LocalTransform; }
            std::string GetBoneName() const { return m_Name; }
            int GetBoneID() { return m_ID; }



            int GetPositionIndex(float animationTime) {
                for (int index = 0; index < m_NumPositions - 1; ++index) {
                    if (animationTime < m_Positions[index + 1].timeStamp)
                        return index;
                }
                assert(0);
            }

            int GetRotationIndex(float animationTime) {
                for (int index = 0; index < m_NumRotations - 1; ++index) {
                    if (animationTime < m_Rotations[index + 1].timeStamp)
                        return index;
                }
                assert(0);
            }

            int GetScaleIndex(float animationTime) {
                for (int index = 0; index < m_NumScalings - 1; ++index) {
                    if (animationTime < m_Scales[index + 1].timeStamp)
                        return index;
                }
                assert(0);
            }


        private:

            float GetScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime) {
                float scaleFactor = 0.0f;
                float midWayLength = animationTime - lastTimeStamp;
                float framesDiff = nextTimeStamp - lastTimeStamp;
                scaleFactor = midWayLength / framesDiff;
                return scaleFactor;
            }

            MATHGL::Matrix4 InterpolatePosition(float animationTime) {
                if (1 == m_NumPositions) {
                    return MATHGL::Matrix4::Translation(m_Positions[0].position);// glm::translate(glm::mat4(1.0f), m_Positions[0].position);
                }
                int p0Index = GetPositionIndex(animationTime);
                int p1Index = p0Index + 1;
                float scaleFactor = GetScaleFactor(m_Positions[p0Index].timeStamp,
                    m_Positions[p1Index].timeStamp, animationTime);
                auto finalPosition = MATHGL::Vector3::Mix(m_Positions[p0Index].position, m_Positions[p1Index].position, scaleFactor); // glm::mix(m_Positions[p0Index].position, m_Positions[p1Index].position, scaleFactor);
                return MATHGL::Matrix4::Translation(finalPosition); //glm::translate(glm::mat4(1.0f), finalPosition);
            }
            
            MATHGL::Matrix4 InterpolateRotation(float animationTime) {
                if (1 == m_NumRotations) {
                    return MATHGL::Quaternion::ToMatrix4(MATHGL::Quaternion::Normalize(m_Rotations[0].orientation));
                	auto rotation = glm::normalize(glm::quat(m_Rotations[0].orientation.w, m_Rotations[0].orientation.x, m_Rotations[0].orientation.y, m_Rotations[0].orientation.z));
                    auto res = glm::toMat4(rotation);
                    return MATHGL::Matrix4(
                        res[0][0], res[1][0], res[2][0], res[3][0],
                        res[0][1], res[1][1], res[2][1], res[3][1],
                        res[0][2], res[1][2], res[2][2], res[3][2],
                        res[0][3], res[1][3], res[2][3], res[3][3]
                    );
                }

                int p0Index = GetRotationIndex(animationTime);
                int p1Index = p0Index + 1;
                float scaleFactor = GetScaleFactor(m_Rotations[p0Index].timeStamp, m_Rotations[p1Index].timeStamp, animationTime);
                auto res = MATHGL::Quaternion::Slerp(m_Rotations[p0Index].orientation, m_Rotations[p1Index].orientation, scaleFactor);
                return MATHGL::Quaternion::ToMatrix4(MATHGL::Quaternion::Normalize(res));
            }

            MATHGL::Matrix4 Bone::InterpolateScaling(float animationTime) {
                if (1 == m_NumScalings)
                    return MATHGL::Matrix4::Scaling(m_Scales[0].scale);// glm::scale(glm::mat4(1.0f), m_Scales[0].scale);

                int p0Index = GetScaleIndex(animationTime);
                int p1Index = p0Index + 1;
                float scaleFactor = GetScaleFactor(m_Scales[p0Index].timeStamp, m_Scales[p1Index].timeStamp, animationTime);
                auto finalScale = MATHGL::Vector3::Mix(m_Scales[p0Index].scale, m_Scales[p1Index].scale, scaleFactor);
                return MATHGL::Matrix4::Scaling(finalScale);
            }

            std::vector<KeyPosition> m_Positions;
            std::vector<KeyRotation> m_Rotations;
            std::vector<KeyScale> m_Scales;
            int m_NumPositions;
            int m_NumRotations;
            int m_NumScalings;

            MATHGL::Matrix4 m_LocalTransform;
            std::string m_Name;
            int m_ID;
        };

        class Animation {
        public:
            Animation() = default;

            Animation(const std::string& animationPath, RENDER::Model* model);

            ~Animation() {
            }

            Bone* FindBone(const std::string& name) {
                auto iter = std::find_if(m_Bones.begin(), m_Bones.end(),
                    [&](const Bone& Bone) {
                    return Bone.GetBoneName() == name;
                }
                );
                if (iter == m_Bones.end()) return nullptr;
                else return &(*iter);
            }


            inline float GetTicksPerSecond() { return m_TicksPerSecond; }
            inline float GetDuration() { return m_Duration; }
            inline const AssimpNodeData& GetRootNode() { return m_RootNode; }
            inline const std::map<std::string, RENDER::BoneInfo>& GetBoneIDMap() {
                return m_BoneInfoMap;
            }
            static std::map<std::string, std::shared_ptr<Animation>> LoadAnimations(const std::string& animationPath, RENDER::Model* model);
        private:
            Animation(const aiAnimation& animation, const aiScene& scene, RENDER::Model& model);

            void ReadMissingBones(const aiAnimation& animation, RENDER::Model& model) {
                int size = animation.mNumChannels;

                auto& boneInfoMap = model.GetBoneInfoMap();//getting m_BoneInfoMap from Model class
                int& boneCount = model.GetBoneCount(); //getting the m_BoneCounter from Model class

                //reading channels(bones engaged in an animation and their keyframes)
                for (int i = 0; i < size; i++) {
                    auto channel = animation.mChannels[i];
                    std::string boneName = channel->mNodeName.data;

                    if (boneInfoMap.find(boneName) == boneInfoMap.end()) {
                        boneInfoMap[boneName].id = boneCount;
                        boneCount++;
                    }
                    m_Bones.push_back(Bone(channel->mNodeName.data,
                        boneInfoMap[channel->mNodeName.data].id, channel));
                }

                m_BoneInfoMap = boneInfoMap;
            }

            void ReadHeirarchyData(AssimpNodeData& dest, const aiNode* src) {
                assert(src);

                dest.name = src->mName.data;
                dest.transformation = AssimpGLMHelpers::ConvertMatrixToGLMFormat(src->mTransformation);
                dest.childrenCount = src->mNumChildren;

                for (int i = 0; i < src->mNumChildren; i++) {
                    AssimpNodeData newData;
                    ReadHeirarchyData(newData, src->mChildren[i]);
                    dest.children.push_back(newData);
                }
            }
            float m_Duration;
            int m_TicksPerSecond;
            std::vector<Bone> m_Bones;
            AssimpNodeData m_RootNode;
            std::map<std::string, RENDER::BoneInfo> m_BoneInfoMap;
        };

        class Animator {
        public:
            Animator::Animator(Animation* animation) {
                m_CurrentTime = 0.0;
                m_CurrentAnimation = animation;

                m_FinalBoneMatrices.reserve(100);

                for (int i = 0; i < 100; i++)
                    m_FinalBoneMatrices.push_back(MATHGL::Matrix4(1.0f));
            }

            void Animator::UpdateAnimation(float dt) {
                if (!m_CurrentAnimation) {
                    return;
                }
                m_DeltaTime = dt;
                if (m_CurrentAnimation) {
                    m_CurrentTime += m_CurrentAnimation->GetTicksPerSecond() * dt;
                    m_CurrentTime = fmod(m_CurrentTime, m_CurrentAnimation->GetDuration());
                    CalculateBoneTransform(&m_CurrentAnimation->GetRootNode(), MATHGL::Matrix4(1.0f));
                }
            }

            void Animator::PlayAnimation(Animation* pAnimation) {
                m_CurrentAnimation = pAnimation;
                m_CurrentTime = 0.0f;
            }

            void Animator::CalculateBoneTransform(const AssimpNodeData* node, MATHGL::Matrix4 parentTransform) {
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
                    int index = boneInfoMap[nodeName].id;
                    auto offset = boneInfoMap[nodeName].offset;
                    m_FinalBoneMatrices[index] = globalTransformation * offset;
                }

                for (int i = 0; i < node->childrenCount; i++) {
                    CalculateBoneTransform(&node->children[i], globalTransformation);
                }
            }

            std::vector<MATHGL::Matrix4> GetFinalBoneMatrices() {
                return m_FinalBoneMatrices;
            }

        private:
            std::vector<MATHGL::Matrix4> m_FinalBoneMatrices;
            Animation* m_CurrentAnimation;
            float m_CurrentTime;
            float m_DeltaTime;

        };
		
	};
};
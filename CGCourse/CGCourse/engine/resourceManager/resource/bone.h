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
            glm::mat4 transformation;
            std::string name;
            int childrenCount;
            std::vector<AssimpNodeData> children;
        };
        class AssimpGLMHelpers {
        public:

            static inline glm::mat4 ConvertMatrixToGLMFormat(const aiMatrix4x4& from) {
                glm::mat4 to;
                //the a,b,c,d in assimp is the row ; the 1,2,3,4 is the column
                to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
                to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
                to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
                to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
                return to;
            }

            static inline glm::vec3 GetGLMVec(const aiVector3D& vec) {
                return glm::vec3(vec.x, vec.y, vec.z);
            }

            static inline glm::quat GetGLMQuat(const aiQuaternion& pOrientation) {
                return glm::quat(pOrientation.w, pOrientation.x, pOrientation.y, pOrientation.z);
            }
        };

        struct KeyPosition {
            glm::vec3 position;
            float timeStamp;
        };

        struct KeyRotation {
            glm::quat orientation;
            float timeStamp;
        };

        struct KeyScale {
            glm::vec3 scale;
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
                glm::mat4 translation = InterpolatePosition(animationTime);
                glm::mat4 rotation = InterpolateRotation(animationTime);
                glm::mat4 scale = InterpolateScaling(animationTime);
                m_LocalTransform = translation * rotation * scale;
            }
            glm::mat4 GetLocalTransform() { return m_LocalTransform; }
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

            glm::mat4 InterpolatePosition(float animationTime) {
                if (1 == m_NumPositions)
                    return glm::translate(glm::mat4(1.0f), m_Positions[0].position);

                int p0Index = GetPositionIndex(animationTime);
                int p1Index = p0Index + 1;
                float scaleFactor = GetScaleFactor(m_Positions[p0Index].timeStamp,
                    m_Positions[p1Index].timeStamp, animationTime);
                glm::vec3 finalPosition = glm::mix(m_Positions[p0Index].position, m_Positions[p1Index].position
                    , scaleFactor);
                return glm::translate(glm::mat4(1.0f), finalPosition);
            }

            glm::mat4 InterpolateRotation(float animationTime) {
                if (1 == m_NumRotations) {
                    auto rotation = glm::normalize(m_Rotations[0].orientation);
                    return glm::toMat4(rotation);
                }

                int p0Index = GetRotationIndex(animationTime);
                int p1Index = p0Index + 1;
                float scaleFactor = GetScaleFactor(m_Rotations[p0Index].timeStamp,
                    m_Rotations[p1Index].timeStamp, animationTime);
                glm::quat finalRotation = glm::slerp(m_Rotations[p0Index].orientation, m_Rotations[p1Index].orientation
                    , scaleFactor);
                finalRotation = glm::normalize(finalRotation);
                return glm::toMat4(finalRotation);

            }

            glm::mat4 Bone::InterpolateScaling(float animationTime) {
                if (1 == m_NumScalings)
                    return glm::scale(glm::mat4(1.0f), m_Scales[0].scale);

                int p0Index = GetScaleIndex(animationTime);
                int p1Index = p0Index + 1;
                float scaleFactor = GetScaleFactor(m_Scales[p0Index].timeStamp,
                    m_Scales[p1Index].timeStamp, animationTime);
                glm::vec3 finalScale = glm::mix(m_Scales[p0Index].scale, m_Scales[p1Index].scale
                    , scaleFactor);
                return glm::scale(glm::mat4(1.0f), finalScale);
            }

            std::vector<KeyPosition> m_Positions;
            std::vector<KeyRotation> m_Rotations;
            std::vector<KeyScale> m_Scales;
            int m_NumPositions;
            int m_NumRotations;
            int m_NumScalings;

            glm::mat4 m_LocalTransform;
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

        private:
            void ReadMissingBones(const aiAnimation* animation, RENDER::Model& model) {
                int size = animation->mNumChannels;

                auto& boneInfoMap = model.GetBoneInfoMap();//getting m_BoneInfoMap from Model class
                int& boneCount = model.GetBoneCount(); //getting the m_BoneCounter from Model class

                //reading channels(bones engaged in an animation and their keyframes)
                for (int i = 0; i < size; i++) {
                    auto channel = animation->mChannels[i];
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
                    m_FinalBoneMatrices.push_back(glm::mat4(1.0f));
            }

            void Animator::UpdateAnimation(float dt) {
                m_DeltaTime = dt;
                if (m_CurrentAnimation) {
                    m_CurrentTime += m_CurrentAnimation->GetTicksPerSecond() * dt;
                    m_CurrentTime = fmod(m_CurrentTime, m_CurrentAnimation->GetDuration());
                    CalculateBoneTransform(&m_CurrentAnimation->GetRootNode(), glm::mat4(1.0f));
                }
            }

            void Animator::PlayAnimation(Animation* pAnimation) {
                m_CurrentAnimation = pAnimation;
                m_CurrentTime = 0.0f;
            }

            void Animator::CalculateBoneTransform(const AssimpNodeData* node, glm::mat4 parentTransform) {
                std::string nodeName = node->name;
                glm::mat4 nodeTransform = node->transformation;

                Bone* Bone = m_CurrentAnimation->FindBone(nodeName);

                if (Bone) {
                    Bone->Update(m_CurrentTime);
                    nodeTransform = Bone->GetLocalTransform();
                }

                glm::mat4 globalTransformation = parentTransform * nodeTransform;

                auto boneInfoMap = m_CurrentAnimation->GetBoneIDMap();
                if (boneInfoMap.find(nodeName) != boneInfoMap.end()) {
                    int index = boneInfoMap[nodeName].id;
                    glm::mat4 offset = boneInfoMap[nodeName].offset;
                    m_FinalBoneMatrices[index] = globalTransformation * offset;
                }

                for (int i = 0; i < node->childrenCount; i++)
                    CalculateBoneTransform(&node->children[i], globalTransformation);
            }

            std::vector<glm::mat4> GetFinalBoneMatrices() {
                return m_FinalBoneMatrices;
            }

        private:
            std::vector<glm::mat4> m_FinalBoneMatrices;
            Animation* m_CurrentAnimation;
            float m_CurrentTime;
            float m_DeltaTime;

        };
		
	};
};
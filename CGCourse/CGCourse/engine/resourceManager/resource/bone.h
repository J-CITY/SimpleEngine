#pragma once
#include <vector>

#include "../../render/Model.h"
import glmath;
//#include <glm/gtx/quaternion.hpp>
//#include <glm/glm.hpp>

struct aiNodeAnim;

namespace KUMA {
	namespace RENDER {
        class Model;
	}
	namespace RESOURCES {
        struct AssimpNodeData {
            MATHGL::Matrix4 transformation;
            std::string name;
            int childrenCount;
            std::vector<AssimpNodeData> children;
        };
        static MATHGL::Matrix4 ConvertMatrix4x4(const aiMatrix4x4& from) {
            MATHGL::Matrix4 to;
            //the a,b,c,d in assimp is the row ; the 1,2,3,4 is the column
            to(0, 0) = from.a1; to(1, 0) = from.b1; to(2, 0) = from.c1; to(3, 0) = from.d1;
            to(0, 1) = from.a2; to(1, 1) = from.b2; to(2, 1) = from.c2; to(3, 1) = from.d2;
            to(0, 2) = from.a3; to(1, 2) = from.b3; to(2, 2) = from.c3; to(3, 2) = from.d3;
            to(0, 3) = from.a4; to(1, 3) = from.b4; to(2, 3) = from.c4; to(3, 3) = from.d4;
            return to;
        }
        static MATHGL::Vector3 ConvertVector3(const aiVector3D& vec) {
            return MATHGL::Vector3(vec.x, vec.y, vec.z);
        }
        static MATHGL::Quaternion ConvertQuaternion(const aiQuaternion& pOrientation) {
            return MATHGL::Quaternion(pOrientation.x, pOrientation.y, pOrientation.z, pOrientation.w);
        }

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
            Bone(const std::string& name, int ID, const aiNodeAnim* channel);
            void Update(float animationTime);
            MATHGL::Matrix4 GetLocalTransform() { return m_LocalTransform; }
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
            Animation();
            Animation(const std::string& animationPath, RENDER::Model* model);
            ~Animation();

            Bone* FindBone(const std::string& name);
            float GetTicksPerSecond();
            float GetDuration();
            const AssimpNodeData& GetRootNode();
            const std::map<std::string, RENDER::BoneInfo>& GetBoneIDMap();
            static std::map<std::string, std::shared_ptr<Animation>> LoadAnimations(const std::string& animationPath, RENDER::Model* model);
        private:
            Animation(const aiAnimation& animation, const aiScene& scene, RENDER::Model& model);
            void ReadMissingBones(const aiAnimation& animation, RENDER::Model& model);
            void ReadHeirarchyData(AssimpNodeData& dest, const aiNode* src);

            float m_Duration;
            int m_TicksPerSecond;
            std::vector<Bone> m_Bones;
            AssimpNodeData m_RootNode;
            std::map<std::string, RENDER::BoneInfo> m_BoneInfoMap;
        };

        class Animator {
        public:
            Animator(Animation* animation);
            void UpdateAnimation(float dt);
            void PlayAnimation(Animation* pAnimation);
            void CalculateBoneTransform(const AssimpNodeData* node, MATHGL::Matrix4 parentTransform);
            std::vector<MATHGL::Matrix4> GetFinalBoneMatrices();
        private:
            std::vector<MATHGL::Matrix4> m_FinalBoneMatrices;
            Animation* m_CurrentAnimation;
            float m_CurrentTime;
            float m_DeltaTime;

        };
		
	};
};
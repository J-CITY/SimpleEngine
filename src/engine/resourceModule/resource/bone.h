#pragma once
#include <string>
#include <vector>
#include <assimp/scene.h>

#include <renderModule/backends/interface/modelInterface.h>

//#include <glm/gtx/quaternion.hpp>
//#include <glm/glm.hpp>

struct aiNodeAnim;

namespace TRIANGULATION
{
	
}

namespace IKIGAI {
	class BlenderContext;

	namespace RESOURCES {
        struct AssimpNodeData {
            MATH::Matrix4f transformation;
            std::string name;
            int childrenCount;
            std::vector<AssimpNodeData> children;
        };
        static MATH::Matrix4f ConvertMatrix4x4(const aiMatrix4x4& from) {
            MATH::Matrix4f to;
            //the a,b,c,d in assimp is the row ; the 1,2,3,4 is the column
            to(0, 0) = from.a1; to(1, 0) = from.b1; to(2, 0) = from.c1; to(3, 0) = from.d1;
            to(0, 1) = from.a2; to(1, 1) = from.b2; to(2, 1) = from.c2; to(3, 1) = from.d2;
            to(0, 2) = from.a3; to(1, 2) = from.b3; to(2, 2) = from.c3; to(3, 2) = from.d3;
            to(0, 3) = from.a4; to(1, 3) = from.b4; to(2, 3) = from.c4; to(3, 3) = from.d4;
            return to;
        }
        static MATH::Vector3f ConvertVector3(const aiVector3D& vec) {
            return MATH::Vector3f(vec.x, vec.y, vec.z);
        }
        static MATH::QuaternionF ConvertQuaternion(const aiQuaternion& pOrientation) {
            return MATH::QuaternionF(pOrientation.x, pOrientation.y, pOrientation.z, pOrientation.w);
        }

        struct KeyPosition {
            MATH::Vector3f position;
            float timeStamp;
        };

        struct KeyRotation {
            MATH::QuaternionF orientation;
            float timeStamp;
        };

        struct KeyScale {
            MATH::Vector3f scale;
            float timeStamp;
        };

        class Bone {
        public:
            Bone(const std::string& name, int ID, const aiNodeAnim* channel);
            void Update(float animationTime);
            MATH::Matrix4f GetLocalTransform() { return m_LocalTransform; }
            std::string GetBoneName() const;
            int GetBoneID();
            int GetPositionIndex(float animationTime);
            int GetRotationIndex(float animationTime);
            int GetScaleIndex(float animationTime);
        private:
            float GetScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime);
            MATH::Matrix4f InterpolatePosition(float animationTime);
            MATH::Matrix4f InterpolateRotation(float animationTime);
            MATH::Matrix4f InterpolateScaling(float animationTime);

            std::vector<KeyPosition> m_Positions;
            std::vector<KeyRotation> m_Rotations;
            std::vector<KeyScale> m_Scales;
            int m_NumPositions;
            int m_NumRotations;
            int m_NumScalings;

            MATH::Matrix4f m_LocalTransform;
            std::string m_Name;
            int m_ID;
        };

        class Animation {
        public:
            Animation();
            Animation(const std::string& animationPath, RENDER::ModelInterface* model);
            ~Animation();

            Bone* FindBone(const std::string& name);
            float GetTicksPerSecond();
            float GetDuration();
            const AssimpNodeData& GetRootNode();
            const std::map<std::string, RENDER::BoneInfo>& GetBoneIDMap();
            static std::map<std::string, std::shared_ptr<Animation>> LoadAnimations(const std::string& animationPath, RENDER::ModelInterface* model);
            //TODO: remove it;
            RENDER::ModelInterface* model;
        private:
            Animation(const aiAnimation& animation, const aiScene& scene, RENDER::ModelInterface& model);
            void ReadMissingBones(const aiAnimation& animation, RENDER::ModelInterface& model);
            void ReadHeirarchyData(AssimpNodeData& dest, const aiNode* src);

            float m_Duration;
            int m_TicksPerSecond;
            std::vector<Bone> m_Bones;
            AssimpNodeData m_RootNode;
            std::map<std::string, RENDER::BoneInfo> m_BoneInfoMap;
            
        };

        class Blander
        {
        public:
            std::unique_ptr<IKIGAI::BlenderContext> context;

            Blander();

            void setPoints(const std::vector<MATH::Vector2f>& points, const std::vector<std::string>& animNames);

            struct AnimWithBlendInfo {
                std::string name;
                float factor = 0.0f;
            };
            std::vector<AnimWithBlendInfo> getAnimationForBlending(MATH::Vector2f point);

            MATH::Vector2f hor = {0.0f, 1.0f};
            MATH::Vector2f ver = {0.0f, 1.0f};
            MATH::Vector2f point;
            
            std::map<std::string, Animation*> m_Animations;
        };

        class Animator {
        public:
            Animator(Animation* animation);
            void UpdateAnimation(float dt);
            void PlayAnimation(Animation* pAnimation);
            void CalculateBoneTransform(const AssimpNodeData* node, MATH::Matrix4f parentTransform);
            void BlendTwoAnimations(Animation* pBaseAnimation, Animation* pLayeredAnimation, float blendFactor,
                                    float deltaTime);
            void CalculateBlendTwoBoneTransform(Animation* pAnimationBase, const AssimpNodeData* node,
                                                Animation* pAnimationLayer,
                                                const AssimpNodeData* nodeLayered, float currentTimeBase,
                                                float currentTimeLayered,
                                                const MATH::Matrix4f& parentTransform, float blendFactor);
            void BlendThreeAnimations(Animation* pBaseAnimation, Animation* pLayeredAnimation1, Animation* pLayeredAnimation2, float blendFactor1, float blendFactor2, float blendFactor3, float deltaTime);
            void CalculateBlendThreeBoneTransform(
                Animation* pAnimationBase, const AssimpNodeData* node,
                Animation* pAnimationLayer1, const AssimpNodeData* nodeLayered1,
                Animation* pAnimationLayer2, const AssimpNodeData* nodeLayered2,
                const float currentTimeBase, const float currentTimeLayered1, const float currentTimeLayered2,
                const MATH::Matrix4f& parentTransform,
                const float blendFactor1, const float blendFactor2, const float blendFactor3
            );
            std::vector<MATH::Matrix4f> GetFinalBoneMatrices();
            Blander* blender;
        private:
            std::vector<MATH::Matrix4f> m_FinalBoneMatrices;
            Animation* m_CurrentAnimation;
            
            float m_CurrentTime;
            float m_DeltaTime;

        };
		
	};
};
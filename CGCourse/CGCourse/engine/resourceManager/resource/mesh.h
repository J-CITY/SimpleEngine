#pragma once
#include <memory>
#include <string>
#include <vector>
#include <limits>
#include <algorithm>
#include <cassert>
#include <map>
#include <assimp/scene.h>



#include "../../render/buffers/indexBuffer.h"
#include "../../render/buffers/vertexArray.h"
#include "../../render/buffers/vertexBuffer.h"
#include "../../render/objects/BoundingSphere.h"
#include "../../utils/vertex.h"
#include "../../utils/math/Matrix4.h"


#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

namespace  Assimp {
	class Importer;
}

namespace KUMA {
	namespace RESOURCES {
		struct V {
			// position
			glm::vec3 Position;
			// normal
			glm::vec3 Normal;
			// texCoords
			glm::vec2 TexCoords;

			// tangent
			glm::vec3 Tangent;
			// bitangent
			glm::vec3 Bitangent;

			//bone indexes which will influence this vertex
			int m_BoneIDs[MAX_BONE_INFLUENCE];
			//weights from each bone
			float m_Weights[MAX_BONE_INFLUENCE];

		};
		class Mesh {
		public:
			Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned>& indices, unsigned materialIndex):
			vertexCount(static_cast<unsigned>(vertices.size())),
			indicesCount(static_cast<unsigned>(indices.size())),
			materialIndex(materialIndex) {
				createBuffers(vertices, indices);
				computeBoundingSphere(vertices);
			}

			~Mesh() = default;
			
			virtual void bind() {
				//indexBuffer->Bind();
				vertexArray.bind();
			}

			virtual void unbind() {

				//indexBuffer->Unbind();
				vertexArray.unbind();
			}

			virtual unsigned getVertexCount() {
				return vertexCount;
			}

			virtual unsigned getIndexCount() {
				return indicesCount;
			}

			uint32_t getMaterialIndex() const {
				return materialIndex;
			}

			const RENDER::BoundingSphere& getBoundingSphere() const {
				return boundingSphere;
			}

			/*void boneTransform(float TimeInSeconds, std::vector<MATHGL::Matrix4>& Transforms) {
				if (!m_pScene->mAnimations) {
					return;
				}
				MATHGL::Matrix4 Identity;
				
				float TicksPerSecond = m_pScene->mAnimations[0]->mTicksPerSecond != 0 ?
					m_pScene->mAnimations[0]->mTicksPerSecond : 25.0f;

				float TimeInTicks = TimeInSeconds * TicksPerSecond;
				float AnimationTime = fmod(TimeInTicks, m_pScene->mAnimations[0]->mDuration);

				readNodeHeirarchy(AnimationTime, m_pScene->mRootNode, Identity);

				Transforms.resize(m_NumBones);

				for (unsigned i = 0; i < m_NumBones; i++) {
					Transforms[i] = m_BoneInfo[i].FinalTransformation;
				}
			}
			const aiNodeAnim* findNodeAnim(const aiAnimation* pAnimation, const std::string NodeName) {
				for (unsigned i = 0; i < pAnimation->mNumChannels; i++) {
					const aiNodeAnim* pNodeAnim = pAnimation->mChannels[i];

					if (std::string(pNodeAnim->mNodeName.data) == NodeName) {
						return pNodeAnim;
					}
				}

				return nullptr;
			}

			unsigned findScaling(float AnimationTime, const aiNodeAnim* pNodeAnim) {
				assert(pNodeAnim->mNumScalingKeys > 0);

				for (unsigned i = 0; i < pNodeAnim->mNumScalingKeys - 1; i++) {
					if (AnimationTime < (float)pNodeAnim->mScalingKeys[i + 1].mTime) {
						return i;
					}
				}

				assert(0);

				return 0;
			}

			void calcInterpolatedScaling(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim) {
				if (pNodeAnim->mNumScalingKeys == 1) {
					Out = pNodeAnim->mScalingKeys[0].mValue;
					return;
				}

				unsigned ScalingIndex = findScaling(AnimationTime, pNodeAnim);
				unsigned NextScalingIndex = (ScalingIndex + 1);
				//assert(NextScalingIndex < pNodeAnim->mNumScalingKeys);
				float DeltaTime = (float)(pNodeAnim->mScalingKeys[NextScalingIndex].mTime - pNodeAnim->mScalingKeys[ScalingIndex].mTime);
				float Factor = (AnimationTime - (float)pNodeAnim->mScalingKeys[ScalingIndex].mTime) / DeltaTime;
				//assert(Factor >= 0.0f && Factor <= 1.0f);
				const aiVector3D& Start = pNodeAnim->mScalingKeys[ScalingIndex].mValue;
				const aiVector3D& End = pNodeAnim->mScalingKeys[NextScalingIndex].mValue;
				aiVector3D Delta = End - Start;
				Out = Start + Factor * Delta;
			}

			unsigned findPosition(float AnimationTime, const aiNodeAnim* pNodeAnim) {
				for (unsigned i = 0; i < pNodeAnim->mNumPositionKeys - 1; i++) {
					if (AnimationTime < (float)pNodeAnim->mPositionKeys[i + 1].mTime) {
						return i;
					}
				}

				assert(0);

				return 0;
			}

			
			void calcInterpolatedPosition(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim) {
				if (pNodeAnim->mNumPositionKeys == 1) {
					Out = pNodeAnim->mPositionKeys[0].mValue;
					return;
				}

				unsigned PositionIndex = findPosition(AnimationTime, pNodeAnim);
				unsigned NextPositionIndex = (PositionIndex + 1);
				assert(NextPositionIndex < pNodeAnim->mNumPositionKeys);
				float DeltaTime = (float)(pNodeAnim->mPositionKeys[NextPositionIndex].mTime - pNodeAnim->mPositionKeys[PositionIndex].mTime);
				float Factor = (AnimationTime - (float)pNodeAnim->mPositionKeys[PositionIndex].mTime) / DeltaTime;
				assert(Factor >= 0.0f && Factor <= 1.0f);
				const aiVector3D& Start = pNodeAnim->mPositionKeys[PositionIndex].mValue;
				const aiVector3D& End = pNodeAnim->mPositionKeys[NextPositionIndex].mValue;
				aiVector3D Delta = End - Start;
				Out = Start + Factor * Delta;
			}

			unsigned findRotation(float AnimationTime, const aiNodeAnim* pNodeAnim) {
				assert(pNodeAnim->mNumRotationKeys > 0);

				for (unsigned i = 0; i < pNodeAnim->mNumRotationKeys - 1; i++) {
					if (AnimationTime < (float)pNodeAnim->mRotationKeys[i + 1].mTime) {
						return i;
					}
				}

				assert(0);

				return 0;
			}
			
			void calcInterpolatedRotation(aiQuaternion& Out, float AnimationTime, const aiNodeAnim* pNodeAnim) {
				// we need at least two values to interpolate...
				if (pNodeAnim->mNumRotationKeys == 1) {
					Out = pNodeAnim->mRotationKeys[0].mValue;
					return;
				}

				unsigned RotationIndex = findRotation(AnimationTime, pNodeAnim);
				unsigned NextRotationIndex = (RotationIndex + 1);
				assert(NextRotationIndex < pNodeAnim->mNumRotationKeys);
				float DeltaTime = (float)(pNodeAnim->mRotationKeys[NextRotationIndex].mTime - pNodeAnim->mRotationKeys[RotationIndex].mTime);
				float Factor = (AnimationTime - (float)pNodeAnim->mRotationKeys[RotationIndex].mTime) / DeltaTime;
				assert(Factor >= 0.0f && Factor <= 1.0f);
				const aiQuaternion& StartRotationQ = pNodeAnim->mRotationKeys[RotationIndex].mValue;
				const aiQuaternion& EndRotationQ = pNodeAnim->mRotationKeys[NextRotationIndex].mValue;
				aiQuaternion::Interpolate(Out, StartRotationQ, EndRotationQ, Factor);
				Out = Out.Normalize();
			}
			
			void readNodeHeirarchy(float AnimationTime, const aiNode* pNode, const MATHGL::Matrix4& ParentTransform) {
				std::string NodeName(pNode->mName.data);

				const aiAnimation* pAnimation = m_pScene->mAnimations[0];


				MATHGL::Matrix4 NodeTransformation(
					pNode->mTransformation.a1, pNode->mTransformation.a2, pNode->mTransformation.a3, pNode->mTransformation.a4,
					pNode->mTransformation.b1, pNode->mTransformation.b2, pNode->mTransformation.b3, pNode->mTransformation.b4,
					pNode->mTransformation.c1, pNode->mTransformation.c2, pNode->mTransformation.c3, pNode->mTransformation.c4,
					pNode->mTransformation.d1, pNode->mTransformation.d2, pNode->mTransformation.d3, pNode->mTransformation.d4
				);
				const aiNodeAnim* pNodeAnim = findNodeAnim(pAnimation, NodeName);

				if (pNodeAnim) {
					// Интерполируем масштабирование и генерируем матрицу преобразования масштаба
					aiVector3D Scaling;
					calcInterpolatedScaling(Scaling, AnimationTime, pNodeAnim);
					MATHGL::Matrix4 ScalingM;
					ScalingM.Scaling(MATHGL::Vector3(Scaling.x, Scaling.y, Scaling.z));

					// Интерполируем вращение и генерируем матрицу вращения
					aiQuaternion RotationQ;
					calcInterpolatedRotation(RotationQ, AnimationTime, pNodeAnim);
					auto m = RotationQ.GetMatrix();
					MATHGL::Matrix4 RotationM = MATHGL::Matrix4(
						m.a1, m.a2, m.a3, 0,
						m.b1, m.b2, m.b3, 0,
						m.c1, m.c2, m.c3, 0,
						0, 0, 0, 1);

					//  Интерполируем смещение и генерируем матрицу смещения
					aiVector3D Translation;
					calcInterpolatedPosition(Translation, AnimationTime, pNodeAnim);
					MATHGL::Matrix4 TranslationM;
					TranslationM.Translation(MATHGL::Vector3(Translation.x, Translation.y, Translation.z));

					// Объединяем преобразования
					NodeTransformation = TranslationM * RotationM * ScalingM;
				}

				MATHGL::Matrix4 GlobalTransformation = ParentTransform * NodeTransformation;

				if (m_BoneMapping.find(NodeName) != m_BoneMapping.end()) {
					unsigned BoneIndex = m_BoneMapping[NodeName];

					m_BoneInfo[BoneIndex].FinalTransformation = m_GlobalInverseTransform *
						GlobalTransformation *
						m_BoneInfo[BoneIndex].BoneOffset;
				}

				for (unsigned i = 0; i < pNode->mNumChildren; i++) {
					readNodeHeirarchy(AnimationTime, pNode->mChildren[i], GlobalTransformation);
				}
			}*/


			//std::map<std::string, unsigned> m_BoneMapping; // maps a bone name to its index
			//unsigned m_NumBones;
			//std::vector<BoneInfo> m_BoneInfo;
			//MATHGL::Matrix4 m_GlobalInverseTransform;
			//const aiScene* m_pScene=nullptr;
			//std::shared_ptr<Assimp::Importer> imp;

			//int BaseVertex = 0;
			//int BaseIndex = 0;
			
			//unsigned int bounseCount;
			//std::unique_ptr<RENDER::VertexAttribBuffer<VertexBoneData>>			bonesBuffre;
		private:
			void createBuffers(const std::vector<Vertex>& p_vertices, const std::vector<uint32_t>& p_indices);
			void computeBoundingSphere(const std::vector<Vertex>& vertices);

		public:
			const unsigned int vertexCount;
			const unsigned int indicesCount;
			const unsigned int materialIndex;

			RENDER::VertexArray							vertexArray;
			std::unique_ptr<RENDER::VertexBuffer<V>>	vertexBuffer;
			std::unique_ptr<RENDER::IndexBuffer>			indexBuffer;

			RENDER::BoundingSphere boundingSphere;
		};
	}
}

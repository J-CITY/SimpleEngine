#pragma once
#include <memory>
#include <vector>
#include <assimp/scene.h>

#include "../../render/buffers/indexBuffer.h"
#include "../../render/buffers/vertexArray.h"
#include "../../render/buffers/vertexBuffer.h"
#include "../../render/objects/boundingSphere.h"
#include "../../utils/vertex.h"
import glmath;

namespace  Assimp {
	class Importer;
}

namespace KUMA {
	namespace RESOURCES {
		//struct V {
		//	// position
		//	glm::vec3 Position;
		//	// normal
		//	glm::vec3 Normal;
		//	// texCoords
		//	glm::vec2 TexCoords;
		//
		//	// tangent
		//	glm::vec3 Tangent;
		//	// bitangent
		//	glm::vec3 Bitangent;
		//
		//	//bone indexes which will influence this vertex
		//	int m_BoneIDs[MAX_BONE_INFLUENCE];
		//	//weights from each bone
		//	float m_Weights[MAX_BONE_INFLUENCE];
		//
		//};
		class Mesh {
		public:
			Mesh(std::span<Vertex> p_vertices, std::span<unsigned> indices, unsigned materialIndex);
			virtual ~Mesh();
			virtual void bind() const;
			virtual void unbind() const;
			virtual unsigned getVertexCount() const;
			virtual unsigned getIndexCount() const;
			uint32_t getMaterialIndex() const;
			const RENDER::BoundingSphere& getBoundingSphere() const;
		private:
			void createBuffers(std::span<Vertex> p_vertices, std::span<uint32_t> p_indices);
			void computeBoundingSphere(std::span<Vertex> vertices);
		public:
			const unsigned int vertexCount;
			const unsigned int indicesCount;
			const unsigned int materialIndex;

			RENDER::VertexArray vertexArray;
			//std::unique_ptr<RENDER::VertexBuffer<V>> vertexBuffer;
			std::unique_ptr<RENDER::VertexBuffer<Vertex>> vertexBuffer;
			std::unique_ptr<RENDER::IndexBuffer> indexBuffer;
			RENDER::BoundingSphere boundingSphere;
		};
	}
}

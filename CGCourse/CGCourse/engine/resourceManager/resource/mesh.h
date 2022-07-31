#pragma once
#include <memory>
#include <vector>
#include <assimp/scene.h>

#include "../../render/buffers/indexBuffer.h"
#include "../../render/buffers/vertexArray.h"
#include "../../render/buffers/vertexBuffer.h"
#include "../../render/objects/BoundingSphere.h"
#include "../../utils/vertex.h"
import glmath;


#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

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
			Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned>& indices, unsigned materialIndex);
			virtual ~Mesh();
			virtual void bind() const;
			virtual void unbind() const;
			virtual unsigned getVertexCount() const;
			virtual unsigned getIndexCount() const;
			uint32_t getMaterialIndex() const;
			const RENDER::BoundingSphere& getBoundingSphere() const;
		private:
			void createBuffers(const std::vector<Vertex>& p_vertices, const std::vector<uint32_t>& p_indices);
			void computeBoundingSphere(const std::vector<Vertex>& vertices);
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

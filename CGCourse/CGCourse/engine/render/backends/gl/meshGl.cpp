#include "meshGl.h"

#ifdef OPENGL_BACKEND
#include "indexBufferGl.h"


using namespace KUMA;
using namespace KUMA::RENDER;


MeshGl::MeshGl(std::span<Vertex> vertices, std::span<unsigned> indices, unsigned materialIndex) :
	mVertexCount(static_cast<unsigned>(vertices.size())),
	mIndicesCount(static_cast<unsigned>(indices.size())),
	mMaterialIndex(materialIndex) {
	createBuffers(vertices, indices);
	computeBoundingSphere(vertices);
}

MeshGl::MeshGl(std::span<Vertex> vertices, std::span<unsigned> indices, size_t offset, unsigned materialIndex) :
	mVertexCount(static_cast<unsigned>(vertices.size())),
	mIndicesCount(static_cast<unsigned>(indices.size())),
	mMaterialIndex(materialIndex),
	mOffset(offset) {
	computeBoundingSphere(vertices);
}

MeshGl::~MeshGl() = default;

void MeshGl::bind() const {
	//indexBuffer->Bind();
	mVertexArray->bind();
}

void MeshGl::unbind() const {
	//indexBuffer->Unbind();
	mVertexArray->unbind();
}

size_t MeshGl::getVertexCount() const {
	return mVertexCount;
}

size_t MeshGl::getIndexCount() const {
	return mIndicesCount;
}

uint32_t MeshGl::getMaterialIndex() const {
	return mMaterialIndex;
}

void MeshGl::createBuffers(std::span<Vertex> p_vertices, std::span<uint32_t> p_indices) {
	//std::vector<V> vertexData;

	//std::vector<unsigned int> rawIndices;

	//for (const auto& vertex : p_vertices) {
	//	V v;
	//	v.Position = glm::vec3(vertex.position.x, vertex.position.y, vertex.position.z);
	//	v.TexCoords = glm::vec2(vertex.texCoord.x, vertex.texCoord.y);
	//	v.Normal = glm::vec3(vertex.normal.x, vertex.normal.y, vertex.normal.z);
	//	v.Tangent = glm::vec3(vertex.tangent.x, vertex.tangent.y, vertex.tangent.z);
	//	v.Bitangent = glm::vec3(vertex.bitangent.x, vertex.bitangent.y, vertex.bitangent.z);
	//	
	//	//Bone
	//	v.m_BoneIDs[0] = vertex.m_BoneIDs[0];
	//	v.m_BoneIDs[1] = vertex.m_BoneIDs[1];
	//	v.m_BoneIDs[2] = vertex.m_BoneIDs[2];
	//	v.m_BoneIDs[3] = vertex.m_BoneIDs[3];
	//	//
	//	v.m_Weights[0] = vertex.m_Weights[0];
	//	v.m_Weights[1] = vertex.m_Weights[1];
	//	v.m_Weights[2] = vertex.m_Weights[2];
	//	v.m_Weights[3] = vertex.m_Weights[3];
	//
	//	vertexData.push_back(v);
	//}
	mVertexArray = std::make_unique<VertexArray>();
	mVertexBuffer = std::make_unique<VertexBufferGl<Vertex>> (p_vertices);
	mIndexBuffer = std::make_unique<IndexBufferGl>(p_indices);

	//uint64_t vertexSize = sizeof(V);// sizeof(Vertex);
	//vertexArray.bindAttribute(0, *vertexBuffer, RENDER::VertexArray::Type::FLOAT, 3, vertexSize, (intptr_t)offsetof(V, Position));
	//vertexArray.bindAttribute(1, *vertexBuffer, RENDER::VertexArray::Type::FLOAT, 2, vertexSize, (intptr_t)offsetof(V, TexCoords));
	//vertexArray.bindAttribute(2, *vertexBuffer, RENDER::VertexArray::Type::FLOAT, 3, vertexSize, (intptr_t)offsetof(V, Normal));
	//vertexArray.bindAttribute(3, *vertexBuffer, RENDER::VertexArray::Type::FLOAT, 3, vertexSize, (intptr_t)offsetof(V, Tangent));
	//vertexArray.bindAttribute(4, *vertexBuffer, RENDER::VertexArray::Type::FLOAT, 3, vertexSize, (intptr_t)offsetof(V, Bitangent));
	//vertexArray.bindAttribute(5, *vertexBuffer, RENDER::VertexArray::Type::INT,   4, vertexSize, (intptr_t)offsetof(V, m_BoneIDs));
	//vertexArray.bindAttribute(6, *vertexBuffer, RENDER::VertexArray::Type::FLOAT, 4, vertexSize, (intptr_t)offsetof(V, m_Weights));

	uint64_t vertexSize = sizeof(Vertex);
	mVertexArray->bindAttribute(0, *mVertexBuffer, VertexArray::Type::FLOAT, 3, vertexSize, (intptr_t)offsetof(Vertex, position));
	mVertexArray->bindAttribute(1, *mVertexBuffer, VertexArray::Type::FLOAT, 2, vertexSize, (intptr_t)offsetof(Vertex, texCoord));
	mVertexArray->bindAttribute(2, *mVertexBuffer, VertexArray::Type::FLOAT, 3, vertexSize, (intptr_t)offsetof(Vertex, normal));
	mVertexArray->bindAttribute(3, *mVertexBuffer, VertexArray::Type::FLOAT, 3, vertexSize, (intptr_t)offsetof(Vertex, tangent));
	mVertexArray->bindAttribute(4, *mVertexBuffer, VertexArray::Type::FLOAT, 3, vertexSize, (intptr_t)offsetof(Vertex, bitangent));
	mVertexArray->bindAttribute(5, *mVertexBuffer, VertexArray::Type::INT,   4, vertexSize, (intptr_t)offsetof(Vertex, m_BoneIDs));
	mVertexArray->bindAttribute(6, *mVertexBuffer, VertexArray::Type::FLOAT, 4, vertexSize, (intptr_t)offsetof(Vertex, m_Weights));

}

void MeshGl::computeBoundingSphere(std::span<Vertex> vertices) {
	mBoundingSphere.position = MATHGL::Vector3::Zero;
	mBoundingSphere.radius = 0.0f;

	if (!vertices.empty()) {
		float minX = std::numeric_limits<float>::max();
		float minY = std::numeric_limits<float>::max();
		float minZ = std::numeric_limits<float>::max();

		float maxX = std::numeric_limits<float>::min();
		float maxY = std::numeric_limits<float>::min();
		float maxZ = std::numeric_limits<float>::min();

		for (const auto& vertex : vertices) {
			minX = std::min(minX, vertex.position.x);
			minY = std::min(minY, vertex.position.y);
			minZ = std::min(minZ, vertex.position.z);

			maxX = std::max(maxX, vertex.position.x);
			maxY = std::max(maxY, vertex.position.y);
			maxZ = std::max(maxZ, vertex.position.z);
		}

		mBoundingSphere.position = MATHGL::Vector3{ minX + maxX, minY + maxY, minZ + maxZ } / 2.0f;

		for (const auto& vertex : vertices) {
			const auto& position = reinterpret_cast<const MATHGL::Vector3&>(vertex.position);
			mBoundingSphere.radius = std::max(mBoundingSphere.radius, MATHGL::Vector3::Distance(mBoundingSphere.position, position));
		}
	}
}


#endif
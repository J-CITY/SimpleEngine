#include "mesh.h"

using namespace KUMA;
using namespace KUMA::RESOURCES;


Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned>& indices, unsigned materialIndex):
	vertexCount(static_cast<unsigned>(vertices.size())),
	indicesCount(static_cast<unsigned>(indices.size())),
	materialIndex(materialIndex) {
	createBuffers(vertices, indices);
	computeBoundingSphere(vertices);
}

Mesh::~Mesh() = default;

void Mesh::bind() const {
	//indexBuffer->Bind();
	vertexArray.bind();
}

void Mesh::unbind() const {
	//indexBuffer->Unbind();
	vertexArray.unbind();
}

unsigned Mesh::getVertexCount() const {
	return vertexCount;
}

unsigned Mesh::getIndexCount() const {
	return indicesCount;
}

uint32_t Mesh::getMaterialIndex() const {
	return materialIndex;
}

const RENDER::BoundingSphere& Mesh::getBoundingSphere() const {
	return boundingSphere;
}

void Mesh::createBuffers(const std::vector<Vertex>& p_vertices, const std::vector<uint32_t>& p_indices) {
	std::vector<V> vertexData;

	std::vector<unsigned int> rawIndices;

	for (const auto& vertex : p_vertices) {
		V v;
		v.Position = glm::vec3(vertex.position.x, vertex.position.y, vertex.position.z);
		v.TexCoords = glm::vec2(vertex.texCoord.x, vertex.texCoord.y);
		v.Normal = glm::vec3(vertex.normal.x, vertex.normal.y, vertex.normal.z);
		v.Tangent = glm::vec3(vertex.tangent.x, vertex.tangent.y, vertex.tangent.z);
		v.Bitangent = glm::vec3(vertex.bitangent.x, vertex.bitangent.y, vertex.bitangent.z);
		
		//Bone
		v.m_BoneIDs[0] = vertex.m_BoneIDs[0];
		v.m_BoneIDs[1] = vertex.m_BoneIDs[1];
		v.m_BoneIDs[2] = vertex.m_BoneIDs[2];
		v.m_BoneIDs[3] = vertex.m_BoneIDs[3];
		//
		v.m_Weights[0] = vertex.m_Weights[0];
		v.m_Weights[1] = vertex.m_Weights[1];
		v.m_Weights[2] = vertex.m_Weights[2];
		v.m_Weights[3] = vertex.m_Weights[3];

		vertexData.push_back(v);
	}

	vertexBuffer = std::make_unique<RENDER::VertexBuffer<V>>(vertexData);
	indexBuffer = std::make_unique<RENDER::IndexBuffer>(p_indices);
	/*
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(V), (void*)0);
	// vertex texture coords
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(V), (void*)offsetof(V, TexCoords));
	// vertex normals
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(V), (void*)offsetof(V, Normal));
	// vertex tangent
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(V), (void*)offsetof(V, Tangent));
	// vertex bitangent
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(V), (void*)offsetof(V, Bitangent));
	// ids
	glEnableVertexAttribArray(5);
	glVertexAttribIPointer(5, 4, GL_INT, sizeof(V), (void*)offsetof(V, m_BoneIDs));
	glEnableVertexAttribArray(6);
	glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(V), (void*)offsetof(V, m_Weights));
	glBindVertexArray(0);
	*/
	uint64_t vertexSize = sizeof(V);// sizeof(Vertex);
	vertexArray.bindAttribute(0, *vertexBuffer, RENDER::VertexArray::Type::FLOAT, 3, vertexSize, (intptr_t)0);
	vertexArray.bindAttribute(1, *vertexBuffer, RENDER::VertexArray::Type::FLOAT, 2, vertexSize, (intptr_t)offsetof(V, TexCoords));
	vertexArray.bindAttribute(2, *vertexBuffer, RENDER::VertexArray::Type::FLOAT, 3, vertexSize, (intptr_t)offsetof(V, Normal));
	vertexArray.bindAttribute(3, *vertexBuffer, RENDER::VertexArray::Type::FLOAT, 3, vertexSize, (intptr_t)offsetof(V, Tangent));
	vertexArray.bindAttribute(4, *vertexBuffer, RENDER::VertexArray::Type::FLOAT, 3, vertexSize, (intptr_t)offsetof(V, Bitangent));
	vertexArray.bindAttribute(5, *vertexBuffer, RENDER::VertexArray::Type::INT,   4, vertexSize, (intptr_t)offsetof(V, m_BoneIDs));
	vertexArray.bindAttribute(6, *vertexBuffer, RENDER::VertexArray::Type::FLOAT, 4, vertexSize, (intptr_t)offsetof(V, m_Weights));
}

void Mesh::computeBoundingSphere(const std::vector<Vertex>& vertices) {
	boundingSphere.position = MATHGL::Vector3::Zero;
	boundingSphere.radius = 0.0f;

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

		boundingSphere.position = MATHGL::Vector3{minX + maxX, minY + maxY, minZ + maxZ} / 2.0f;

		for (const auto& vertex : vertices) {
			const auto& position = reinterpret_cast<const MATHGL::Vector3&>(vertex.position);
			boundingSphere.radius = std::max(boundingSphere.radius, MATHGL::Vector3::Distance(boundingSphere.position, position));
		}
	}
}

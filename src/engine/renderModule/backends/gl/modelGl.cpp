#include "modelGl.h"
#ifdef OPENGL_BACKEND

import glmath;

using namespace IKIGAI;
using namespace IKIGAI::RENDER;

#include <utilsModule/vertex.h>

ModelGl::ModelGl(const std::string& path) {
	mPath = path;
}

ModelGl::~ModelGl() = default;

const std::vector<std::shared_ptr<MeshInterface>>& ModelGl::getMeshes() const {
	return mMeshes;
}

const std::vector<std::string>& ModelGl::getMaterialNames() const {
	return mMaterialNames;
}

void ModelGl::createBuffers(std::span<Vertex> p_vertices, std::span<uint32_t> p_indices) {
	vertexArray = std::make_unique<VertexArray>();
	vertexBuffer = std::make_unique<VertexBufferGl<Vertex>>(p_vertices);
	indexBuffer = std::make_unique<IndexBufferGl>(p_indices);

	uint64_t vertexSize = sizeof(Vertex);
	vertexArray->bindAttribute(0, *vertexBuffer, VertexArray::Type::FLOAT, 3, vertexSize, (intptr_t)offsetof(Vertex, position));
	vertexArray->bindAttribute(1, *vertexBuffer, VertexArray::Type::FLOAT, 2, vertexSize, (intptr_t)offsetof(Vertex, texCoord));
	vertexArray->bindAttribute(2, *vertexBuffer, VertexArray::Type::FLOAT, 3, vertexSize, (intptr_t)offsetof(Vertex, normal));
	vertexArray->bindAttribute(3, *vertexBuffer, VertexArray::Type::FLOAT, 3, vertexSize, (intptr_t)offsetof(Vertex, tangent));
	vertexArray->bindAttribute(4, *vertexBuffer, VertexArray::Type::FLOAT, 3, vertexSize, (intptr_t)offsetof(Vertex, bitangent));
	vertexArray->bindAttribute(5, *vertexBuffer, VertexArray::Type::INT,   4, vertexSize, (intptr_t)offsetof(Vertex, m_BoneIDs));
	vertexArray->bindAttribute(6, *vertexBuffer, VertexArray::Type::FLOAT, 4, vertexSize, (intptr_t)offsetof(Vertex, m_Weights));
}

#endif


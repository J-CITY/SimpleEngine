#include "modelGl.h"
#ifdef OPENGL_BACKEND
#include "../../vertex.h"

IKIGAI::RENDER::ModelGl::ModelGl(const std::string& path) {
	mPath = path;
}

IKIGAI::RENDER::ModelGl::ModelGl(const std::string& path, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) {
	mPath = path;
	createBuffers(vertices, indices);
}

IKIGAI::RENDER::ModelGl::~ModelGl() = default;

//const std::vector<std::shared_ptr<IKIGAI::RENDER::MeshInterface>>& IKIGAI::RENDER::ModelGl::getMeshes() const {
//	return mMeshes;
//}
//
//const std::vector<std::string>& IKIGAI::RENDER::ModelGl::getMaterialNames() const {
//	return mMaterialNames;
//}

void IKIGAI::RENDER::ModelGl::createBuffers(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) {
	vertexArray = std::make_unique<VertexArray>();
	vertexBuffer = std::make_unique<VertexBufferGl>(vertices);
	indexBuffer = std::make_unique<IndexBufferGl>(indices);

	uint64_t vertexSize = sizeof(Vertex);
	vertexArray->bindAttribute(0, *vertexBuffer, VertexArray::AttributeType::FLOAT, 3, vertexSize, (intptr_t)offsetof(Vertex, position));
	vertexArray->bindAttribute(1, *vertexBuffer, VertexArray::AttributeType::FLOAT, 2, vertexSize, (intptr_t)offsetof(Vertex, texCoord));
	vertexArray->bindAttribute(2, *vertexBuffer, VertexArray::AttributeType::FLOAT, 3, vertexSize, (intptr_t)offsetof(Vertex, normal));
	vertexArray->bindAttribute(3, *vertexBuffer, VertexArray::AttributeType::FLOAT, 3, vertexSize, (intptr_t)offsetof(Vertex, tangent));
	vertexArray->bindAttribute(4, *vertexBuffer, VertexArray::AttributeType::FLOAT, 3, vertexSize, (intptr_t)offsetof(Vertex, bitangent));
	vertexArray->bindAttribute(5, *vertexBuffer, VertexArray::AttributeType::INT, 4, vertexSize, (intptr_t)offsetof(Vertex, m_BoneIDs));
	vertexArray->bindAttribute(6, *vertexBuffer, VertexArray::AttributeType::FLOAT, 4, vertexSize, (intptr_t)offsetof(Vertex, m_Weights));
}

#endif


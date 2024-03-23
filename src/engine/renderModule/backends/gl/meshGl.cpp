#include "meshGl.h"

#ifdef OPENGL_BACKEND
#include "indexBufferGl.h"


using namespace IKIGAI;
using namespace IKIGAI::RENDER;
#ifdef __ANDROID__
#include <android/log.h>
#endif

MeshGl::MeshGl(std::span<Vertex> vertices, std::span<unsigned> indices, unsigned materialIndex) :
	mVertexCount(static_cast<unsigned>(vertices.size())),
	mIndicesCount(static_cast<unsigned>(indices.size())),
	mMaterialIndex(materialIndex) {

	for (auto& e : indices) {
		//std::string s = std::to_string(e.position.x) + " " + std::to_string(e.position.y) + " " + std::to_string(e.position.z);
		//std::cout << e.position.x << " " << e.position.y << " " << e.position.z << std::endl;
		std::string s = std::to_string(e);
		std::cout << e << std::endl;
#ifdef __ANDROID__
		// Android doesn't emit the standard output streams by default, so instead we will
		// pump the logging through the Android Logcat system. If you enter 'a-simple-triangle'
		// as the Logcat filter you will see only these logging statements.
		__android_log_print(ANDROID_LOG_DEBUG, "a-simple-triangle", "%s: %s", "IKIGAI", s.c_str());
#endif
	}


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

void bindAttribute(unsigned int attribute, VertexArray::Type type, int count, int stride, intptr_t offset) {
	glEnableVertexAttribArray(attribute);
	glVertexAttribPointer(attribute, count, static_cast<GLenum>(type), GL_FALSE, stride, reinterpret_cast<const GLvoid*>(offset));
}

void unbindAttribute(unsigned int attribute) {
	glDisableVertexAttribArray(attribute);
}


void MeshGl::bind() const {
#ifndef USING_GLES
	mVertexArray->bind();
#else
	mVertexBuffer->bind();
	mIndexBuffer->bind();

	const auto vertexSize = sizeof(Vertex);
	bindAttribute(0, VertexArray::Type::FLOAT, 3, vertexSize, (intptr_t)offsetof(Vertex, position));
	bindAttribute(1, VertexArray::Type::FLOAT, 2, vertexSize, (intptr_t)offsetof(Vertex, texCoord));
	bindAttribute(2, VertexArray::Type::FLOAT, 3, vertexSize, (intptr_t)offsetof(Vertex, normal));
	bindAttribute(3, VertexArray::Type::FLOAT, 3, vertexSize, (intptr_t)offsetof(Vertex, tangent));
	bindAttribute(4, VertexArray::Type::FLOAT, 3, vertexSize, (intptr_t)offsetof(Vertex, bitangent));
	bindAttribute(5, VertexArray::Type::FLOAT, 4, vertexSize, (intptr_t)offsetof(Vertex, m_BoneIDs));
	bindAttribute(6, VertexArray::Type::FLOAT, 4, vertexSize, (intptr_t)offsetof(Vertex, m_Weights));
#endif
;
}

void MeshGl::unbind() const {
#ifndef USING_GLES
	mVertexArray->unbind();
#else
	for (int i = 0; i <= 6; ++i) {
		unbindAttribute(i);
	}
	mVertexBuffer->unbind();
	mIndexBuffer->unbind();
#endif
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
#ifndef USING_GLES
	mVertexArray = std::make_unique<VertexArray>();
#endif
	mVertexBuffer = std::make_unique<VertexBufferGl<Vertex>> (p_vertices);
	mIndexBuffer = std::make_unique<IndexBufferGl>(p_indices);
#ifndef USING_GLES
	uint64_t vertexSize = sizeof(Vertex);
	mVertexArray->bindAttribute(0, *mVertexBuffer, VertexArray::Type::FLOAT, 3, vertexSize, (intptr_t)offsetof(Vertex, position));
	mVertexArray->bindAttribute(1, *mVertexBuffer, VertexArray::Type::FLOAT, 2, vertexSize, (intptr_t)offsetof(Vertex, texCoord));
	mVertexArray->bindAttribute(2, *mVertexBuffer, VertexArray::Type::FLOAT, 3, vertexSize, (intptr_t)offsetof(Vertex, normal));
	mVertexArray->bindAttribute(3, *mVertexBuffer, VertexArray::Type::FLOAT, 3, vertexSize, (intptr_t)offsetof(Vertex, tangent));
	mVertexArray->bindAttribute(4, *mVertexBuffer, VertexArray::Type::FLOAT, 3, vertexSize, (intptr_t)offsetof(Vertex, bitangent));
	mVertexArray->bindAttribute(5, *mVertexBuffer, VertexArray::Type::FLOAT,   4, vertexSize, (intptr_t)offsetof(Vertex, m_BoneIDs));
	mVertexArray->bindAttribute(6, *mVertexBuffer, VertexArray::Type::FLOAT, 4, vertexSize, (intptr_t)offsetof(Vertex, m_Weights));
#endif
}

void MeshGl::computeBoundingSphere(std::span<Vertex> vertices) {
	mBoundingSphere.position = MATH::Vector3f::Zero;
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

		mBoundingSphere.position = MATH::Vector3f{ minX + maxX, minY + maxY, minZ + maxZ } / 2.0f;

		for (const auto& vertex : vertices) {
			const auto& position = reinterpret_cast<const MATH::Vector3f&>(vertex.position);
			mBoundingSphere.radius = std::max(mBoundingSphere.radius, MATH::Vector3f::Distance(mBoundingSphere.position, position));
		}
	}
}


#endif
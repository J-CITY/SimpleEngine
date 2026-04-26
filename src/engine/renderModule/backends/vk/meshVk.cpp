#include "meshVk.h"

#ifdef VULKAN_BACKEND
#include "vertexBufferVk.h"
#include "indexBufferVk.h"

using namespace IKIGAI;
using namespace IKIGAI::RENDER;


MeshVk::MeshVk(std::vector<Vertex> vertices, std::vector<unsigned> indices, unsigned materialIndex) :
	mVertexCount(static_cast<unsigned>(vertices.size())),
	mIndicesCount(static_cast<unsigned>(indices.size())),
	mMaterialIndex(materialIndex) {
	createBuffers(vertices, indices);
	computeBoundingSphere(vertices);
}

MeshVk::MeshVk(std::vector<Vertex> vertices, std::vector<unsigned> indices, size_t offset, unsigned materialIndex) :
	mVertexCount(static_cast<unsigned>(vertices.size())),
	mIndicesCount(static_cast<unsigned>(indices.size())),
	mMaterialIndex(materialIndex),
	mOffset(offset) {
	computeBoundingSphere(vertices);
}

MeshVk::~MeshVk() = default;


void MeshVk::unbind() const {
	//mVertexBuffer->unbind();
	//mIndexBuffer->unbind();
}

size_t MeshVk::getVertexCount() const {
	return mVertexCount;
}

size_t MeshVk::getIndexCount() const {
	return mIndicesCount;
}


void MeshVk::createBuffers(std::vector<Vertex> p_vertices, std::vector<uint32_t> p_indices) {
	mVertexBuffer = std::make_unique<VertexBufferVk>(p_vertices);
	mIndexBuffer = std::make_unique<IndexBufferVk>(p_indices);
}

void MeshVk::computeBoundingSphere(std::vector<Vertex> vertices) {
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

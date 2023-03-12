#include "meshVk.h"

#ifdef VULKAN_BACKEND
#include "vertexBufferVk.h"
#include "indexBufferVk.h"

using namespace KUMA;
using namespace KUMA::RENDER;


MeshVk::MeshVk(std::span<Vertex> vertices, std::span<unsigned> indices, unsigned materialIndex) :
	mVertexCount(static_cast<unsigned>(vertices.size())),
	mIndicesCount(static_cast<unsigned>(indices.size())),
	mMaterialIndex(materialIndex) {
	createBuffers(vertices, indices);
	computeBoundingSphere(vertices);
}

MeshVk::MeshVk(std::span<Vertex> vertices, std::span<unsigned> indices, size_t offset, unsigned materialIndex) :
	mVertexCount(static_cast<unsigned>(vertices.size())),
	mIndicesCount(static_cast<unsigned>(indices.size())),
	mMaterialIndex(materialIndex),
	mOffset(offset) {
	computeBoundingSphere(vertices);
}

MeshVk::~MeshVk() = default;

void MeshVk::bind(const ShaderInterface& shader) const {
	mVertexBuffer->bind(shader);
	mIndexBuffer->bind(shader);
}

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

uint32_t MeshVk::getMaterialIndex() const {
	return mMaterialIndex;
}

void MeshVk::createBuffers(std::span<Vertex> p_vertices, std::span<uint32_t> p_indices) {
	mVertexBuffer = std::make_unique<VertexBufferVk<Vertex>>(p_vertices);
	mIndexBuffer = std::make_unique<IndexBufferVk>(p_indices);
}

void MeshVk::computeBoundingSphere(std::span<Vertex> vertices) {
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

#pragma once
#include "interface/vertexLayout.h"
#include "renderModule/objects/boundingSphere.h"
#include "renderModule/vertex.h"
#include <memory>
#include <string>
#include <vector>


namespace IKIGAI::RENDER {
class ModelInterface;
class MeshInterface;
struct MeshData {
	VertexLayout layout;
	std::vector<VertexStreamData> streams;
	std::span<const uint32_t> indices;
	size_t vertexCount = 0;
	// Bounds are explicit: arbitrary vertex bytes need not contain a position.
	BoundingSphere bounds;
	// Optional canonical geometry for the existing RT/skinning scene builder.
	std::span<const Vertex> rayTracingVertices;
};
std::shared_ptr<MeshInterface> CreateMesh(const MeshData& data, unsigned materialIndex = 0);

std::shared_ptr<ModelInterface> CreateEmptyModel(const std::string& path = "");
std::shared_ptr<MeshInterface> CreateMesh(const std::vector<Vertex>& vertices, const std::vector<unsigned>& indices, unsigned materialIndex = 0);
std::shared_ptr<MeshInterface> CreateMesh(
	const std::vector<Vertex>& vertices, const std::vector<unsigned>& indices, size_t offset, unsigned materialIndex);
}

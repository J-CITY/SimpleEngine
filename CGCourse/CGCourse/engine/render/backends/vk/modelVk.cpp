#include "modelVk.h"

#ifdef VULKAN_BACKEND

import glmath;

using namespace KUMA;
using namespace KUMA::RENDER;

#include "indexBufferVk.h"
#include "../../../utils/vertex.h"

ModelVk::ModelVk(const std::string& path) {
	mPath = path;
}

ModelVk::~ModelVk() = default;

const std::vector<std::shared_ptr<MeshInterface>>& ModelVk::getMeshes() const {
	return mMeshes;
}

const std::vector<std::string>& ModelVk::getMaterialNames() const {
	return mMaterialNames;
}

void ModelVk::createBuffers(std::span<Vertex> p_vertices, std::span<uint32_t> p_indices) {
	mVertexBuffer = std::make_unique<VertexBufferVk<Vertex>>(p_vertices);
	mIndexBuffer = std::make_unique<IndexBufferVk>(p_indices);
}


#endif

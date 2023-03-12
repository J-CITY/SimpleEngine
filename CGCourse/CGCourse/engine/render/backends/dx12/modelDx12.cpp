#include "modelDx12.h"

#ifdef DX12_BACKEND

import glmath;

using namespace KUMA;
using namespace KUMA::RENDER;

//#include "indexBufferVk.h"
#include "../../../utils/vertex.h"

ModelDx12::ModelDx12(const std::string& path) {
	mPath = path;
}

ModelDx12::~ModelDx12() = default;

const std::vector<std::shared_ptr<MeshInterface>>& ModelDx12::getMeshes() const {
	return mMeshes;
}

const std::vector<std::string>& ModelDx12::getMaterialNames() const {
	return mMaterialNames;
}

//void ModelDx12::createBuffers(std::span<Vertex> p_vertices, std::span<uint32_t> p_indices) {
//	mVertexBuffer = std::make_unique<VertexBufferVk<Vertex>>(p_vertices);
//	mIndexBuffer = std::make_unique<IndexBufferVk>(p_indices);
//}


#endif

#include "modelDx12.h"

#ifdef DX12_BACKEND

using namespace IKIGAI;
using namespace IKIGAI::RENDER;

//#include "indexBufferVk.h"

ModelDx12::ModelDx12(const std::string& path) {
	mPath = path;
}

ModelDx12::~ModelDx12() = default;



//void ModelDx12::createBuffers(std::span<Vertex> p_vertices, std::span<uint32_t> p_indices) {
//	mVertexBuffer = std::make_unique<VertexBufferVk<Vertex>>(p_vertices);
//	mIndexBuffer = std::make_unique<IndexBufferVk>(p_indices);
//}


#endif

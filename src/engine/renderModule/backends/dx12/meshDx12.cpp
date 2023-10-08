#include "meshDx12.h"
#ifdef DX12_BACKEND
#include <utilsModule/vertex.h>

#include "vertexBufferDx12.h"
#include "indexBufferDx12.h"

using namespace IKIGAI::RENDER;
MeshDx12::MeshDx12(const std::vector<Vertex>& vertices, const std::vector<unsigned>& indices, unsigned materialIndex) {

	vertexBuffer = std::make_shared<VertexBufferDx12<Vertex>>(vertices);
	indexBuffer = std::make_shared<IndexBufferDx12>(indices);
}
#endif

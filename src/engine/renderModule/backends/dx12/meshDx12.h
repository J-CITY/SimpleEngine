#pragma once
#include "renderModule/vertex.h"
#include "renderModule/backends/interface/meshInterface.h"
#ifdef DX12_BACKEND
#include <memory>
#include <vector>

#include "indexBufferDx12.h"
#include "vertexBufferDx12.h"


namespace IKIGAI::RENDER {
	//TODO: add support offset in buffers
	class MeshDx12: public MeshInterface {
	public:
		MeshDx12(const std::vector<Vertex>& vertices, const std::vector<unsigned>& indices, unsigned materialIndex = 0);
		MeshDx12(const std::vector<Vertex>& vertices, unsigned materialIndex = 0);
		void bind() const override;;
		void unbind() const override;;
	private:
		std::shared_ptr<VertexBufferInterface> mVertexBuffer;
		std::shared_ptr<IndexBufferInterface> mIndexBuffer;
	};
}
#endif

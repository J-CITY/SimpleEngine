#pragma once
#ifdef DX12_BACKEND
#include <memory>
#include <vector>

#include "indexBufferDx12.h"
#include "vertexBufferDx12.h"

namespace IKIGAI
{
	class Vertex;
}

namespace IKIGAI::RENDER {
	class MeshDx12: public MeshInterface {
	public:

		MeshDx12(const std::vector<Vertex>& vertices, const std::vector<unsigned>& indices, unsigned materialIndex);
		size_t getIndexCount() const override { return 0; };
		size_t getVertexCount() const override { return 0; };
		void bind() const override{};
		void unbind() const override{};
		uint32_t getMaterialIndex() const override { return 0; };

		std::shared_ptr<VertexBufferInterface> vertexBuffer;
		std::shared_ptr<IndexBufferInterface> indexBuffer;
	};
}
#endif

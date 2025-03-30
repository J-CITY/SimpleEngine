#pragma once
#ifdef OPENGL_BACKEND
#include <memory>
#include <span>
#include <optional>

#include "../../vertex.h"
#include "vertexBufferGl.h"
#include "../interface/meshInterface.h"


namespace IKIGAI
{
	namespace RENDER
	{
		class IndexBufferGl;
		struct BoundingSphere;
		class IndexBuffer;
	}
}

namespace IKIGAI {
	namespace RENDER {
		class MeshGl: public MeshInterface {
		public:
			MeshGl(std::vector<Vertex> p_vertices, std::vector<unsigned> indices, unsigned materialIndex);
			MeshGl(std::vector<Vertex> vertices, std::vector<unsigned> indices, size_t offset, unsigned materialIndex);
			virtual ~MeshGl();
			virtual void bind() const override;
			virtual void unbind() const override;
			//virtual size_t getVertexCount() const override;
			//virtual size_t getIndexCount() const override;
			//uint32_t getMaterialIndex() const override;
		private:
			void createBuffers(std::vector<Vertex> p_vertices, std::vector<uint32_t> p_indices);
			void computeBoundingSphere(std::span<Vertex> vertices);
		public:
			unsigned int mVertexCount = 0;
			const unsigned int mIndicesCount = 0;
			const unsigned int mMaterialIndex = 0;
//#ifndef USING_GLES
			std::unique_ptr<VertexArray> mVertexArray;
//#endif
			std::unique_ptr<VertexBufferGl> mVertexBuffer;
			std::unique_ptr<IndexBufferGl> mIndexBuffer;

			std::optional<size_t> mOffset = std::nullopt;
		};
	}
}
#endif

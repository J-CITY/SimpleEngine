#pragma once
#ifdef OPENGL_BACKEND
#include <memory>
#include <span>
#include <optional>

#include "../engine/utils/vertex.h"
#include "vertexBufferGl.h"
#include "../interface/meshInterface.h"

import glmath;

namespace KUMA
{
	namespace RENDER
	{
		class IndexBufferGl;
		struct BoundingSphere;
		class IndexBuffer;
	}
}

namespace KUMA {
	namespace RENDER {
		class MeshGl: public MeshInterface {
		public:
			MeshGl(std::span<Vertex> p_vertices, std::span<unsigned> indices, unsigned materialIndex);
			MeshGl(std::span<Vertex> vertices, std::span<unsigned> indices, size_t offset, unsigned materialIndex);
			virtual ~MeshGl();
			virtual void bind() const;
			virtual void unbind() const;
			virtual size_t getVertexCount() const;
			virtual size_t getIndexCount() const;
			uint32_t getMaterialIndex() const override;
		private:
			void createBuffers(std::span<Vertex> p_vertices, std::span<uint32_t> p_indices);
			void computeBoundingSphere(std::span<Vertex> vertices);
		public:
			const unsigned int mVertexCount = 0;
			const unsigned int mIndicesCount = 0;
			const unsigned int mMaterialIndex = 0;

			std::unique_ptr<VertexArray> mVertexArray;
			std::unique_ptr<VertexBufferGl<Vertex>> mVertexBuffer;
			std::unique_ptr<IndexBufferGl> mIndexBuffer;

			std::optional<size_t> mOffset = std::nullopt;
		};
	}
}
#endif

#pragma once
#include "renderModule/vertex.h"

#ifdef VULKAN_BACKEND

#include <vulkan/vulkan.h>

#include <memory>
#include <span>

#include <memory>
#include <span>
#include <optional>

#include "vertexBufferVk.h"
#include "../interface/meshInterface.h"

namespace IKIGAI
{
	namespace RENDER
	{
		class IndexBufferVk;
		struct BoundingSphere;
		class IndexBuffer;
	}
}

namespace IKIGAI::RENDER {
	class MeshVk : public MeshInterface {
	public:
		MeshVk(std::span<Vertex> p_vertices, std::span<unsigned> indices, unsigned materialIndex);
		MeshVk(std::span<Vertex> vertices, std::span<unsigned> indices, size_t offset, unsigned materialIndex);
		virtual ~MeshVk();
		virtual void bind(const ShaderInterface& shader) const;
		virtual void unbind() const;
		virtual size_t getVertexCount() const;
		virtual size_t getIndexCount() const;
		uint32_t getMaterialIndex() const override;
	private:
		void createBuffers(std::span<Vertex> p_vertices, std::span<uint32_t> p_indices);
		void computeBoundingSphere(std::span<Vertex> vertices);

	public:
		void bind() const override {};

		const unsigned int mVertexCount = 0;
		const unsigned int mIndicesCount = 0;
		const unsigned int mMaterialIndex = 0;
		
		std::unique_ptr<VertexBufferVk<Vertex>> mVertexBuffer;
		std::unique_ptr<IndexBufferVk> mIndexBuffer;

		std::optional<size_t> mOffset = std::nullopt;
	};
}
#endif

#pragma once

#include <cstdint>
#include <memory>
#include <vector>
#include "../../vertex.h"

#include "../../objects/boundingSphere.h"


namespace IKIGAI
{
	namespace RENDER
	{
            class IndexBufferInterface;

		struct VertexDescriptor {
			struct Member {
				unsigned mType = 0;
				unsigned mCount = 0;
				unsigned mOffset = 0;
			};
			std::vector<Member> mMembers;
		};

		class MeshInterface {
		public:
			virtual ~MeshInterface() = default;

			size_t getIndexCount() const { return mIndicesCount; };
			size_t getVertexCount() const { return mVertexCount; };
			size_t getMaterialIndex() const { return mMaterialIndex; };
			size_t getOffset() const { return mOffset; };

			virtual void bind() const = 0;
			virtual void unbind() const = 0;
            virtual const std::vector<Vertex>& rayTracingVertices() const {static const std::vector<Vertex> empty;return empty;}
            virtual const std::vector<unsigned>& rayTracingIndices() const {static const std::vector<unsigned> empty;return empty;}
            virtual std::shared_ptr<IndexBufferInterface> rayTracingIndexBuffer() const {return {};}

			const IKIGAI::RENDER::BoundingSphere& getBoundingSphere() const;
			void setOffset(size_t offset);

		protected:
			RENDER::BoundingSphere mBoundingSphere;

			size_t mOffset = 0;

			size_t mVertexCount = 0;
			size_t mIndicesCount = 0;
			size_t mMaterialIndex = 0;
		};
	}
}

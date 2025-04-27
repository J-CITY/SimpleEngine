#pragma once

#include <cstdint>

#include "../../objects/boundingSphere.h"


namespace IKIGAI
{
	namespace RENDER
	{

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

#pragma once

#include <cstdint>

#include "../../objects/boundingSphere.h"


namespace KUMA
{
	namespace RENDER
	{
		class MeshInterface {
		public:
			virtual ~MeshInterface() = default;

			virtual size_t getIndexCount() const = 0;
			virtual size_t getVertexCount() const = 0;

			virtual void bind() const = 0;
			virtual void unbind() const = 0;

			const KUMA::RENDER::BoundingSphere& getBoundingSphere() const;
			virtual uint32_t getMaterialIndex() const = 0;
			void setOffset(size_t offset) {
				mOffset = offset;
			}
		protected:
			RENDER::BoundingSphere mBoundingSphere;

			size_t mOffset = 0;
		};
	}
}

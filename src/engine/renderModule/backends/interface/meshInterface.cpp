#include "meshInterface.h"

using namespace IKIGAI::RENDER;

const BoundingSphere& MeshInterface::getBoundingSphere() const {
	return mBoundingSphere;
}

void MeshInterface::setOffset(size_t offset) {
	mOffset = offset;
}

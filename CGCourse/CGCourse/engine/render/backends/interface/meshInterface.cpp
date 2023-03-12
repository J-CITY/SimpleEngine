#include "meshInterface.h"

using namespace KUMA::RENDER;

const BoundingSphere& MeshInterface::getBoundingSphere() const {
	return mBoundingSphere;
}

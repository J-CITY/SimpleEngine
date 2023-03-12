#include "modelInterface.h"

#include "meshInterface.h"

using namespace KUMA;
using namespace KUMA::RENDER;

const BoundingSphere& ModelInterface::getBoundingSphere() const {
	return mBoundingSphere;
}

const std::string& ModelInterface::getPath() {
	return mPath;
}


void ModelInterface::computeBoundingSphere() {
	if (mMeshes.size() == 1) {
		mBoundingSphere = mMeshes[0]->getBoundingSphere();
	}
	else {
		mBoundingSphere.position = MATHGL::Vector3::Zero;
		mBoundingSphere.radius = 0.0f;

		if (!mMeshes.empty()) {
			float minX = std::numeric_limits<float>::max();
			float minY = std::numeric_limits<float>::max();
			float minZ = std::numeric_limits<float>::max();

			float maxX = std::numeric_limits<float>::min();
			float maxY = std::numeric_limits<float>::min();
			float maxZ = std::numeric_limits<float>::min();

			for (const auto& mesh : mMeshes) {
				const auto& boundingSphere = mesh->getBoundingSphere();
				minX = std::min(minX, boundingSphere.position.x - boundingSphere.radius);
				minY = std::min(minY, boundingSphere.position.y - boundingSphere.radius);
				minZ = std::min(minZ, boundingSphere.position.z - boundingSphere.radius);

				maxX = std::max(maxX, boundingSphere.position.x + boundingSphere.radius);
				maxY = std::max(maxY, boundingSphere.position.y + boundingSphere.radius);
				maxZ = std::max(maxZ, boundingSphere.position.z + boundingSphere.radius);
			}

			mBoundingSphere.position = MATHGL::Vector3{ minX + maxX, minY + maxY, minZ + maxZ } / 2.0f;
			mBoundingSphere.radius = MATHGL::Vector3::Distance(mBoundingSphere.position, { minX, minY, minZ });
		}
	}
}


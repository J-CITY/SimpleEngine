#include "Model.h"

import glmath;

using namespace KUMA;
using namespace KUMA::RENDER;


const BoundingSphere& Model::getBoundingSphere() const {
	return boundingSphere;
}

Model::Model(const std::string& path) : path(path) {
	for (auto m : meshes) {
		delete m;
	}
}

Model::~Model() {
	for (auto m : meshes) {
		delete m;
	}
};

void Model::computeBoundingSphere() {
	if (meshes.size() == 1) {
		boundingSphere = meshes[0]->getBoundingSphere();
	}
	else {
		boundingSphere.position = MATHGL::Vector3::Zero;
		boundingSphere.radius = 0.0f;

		if (!meshes.empty()) {
			float minX = std::numeric_limits<float>::max();
			float minY = std::numeric_limits<float>::max();
			float minZ = std::numeric_limits<float>::max();

			float maxX = std::numeric_limits<float>::min();
			float maxY = std::numeric_limits<float>::min();
			float maxZ = std::numeric_limits<float>::min();

			for (const auto& mesh : meshes) {
				const auto& boundingSphere = mesh->getBoundingSphere();
				minX = std::min(minX, boundingSphere.position.x - boundingSphere.radius);
				minY = std::min(minY, boundingSphere.position.y - boundingSphere.radius);
				minZ = std::min(minZ, boundingSphere.position.z - boundingSphere.radius);

				maxX = std::max(maxX, boundingSphere.position.x + boundingSphere.radius);
				maxY = std::max(maxY, boundingSphere.position.y + boundingSphere.radius);
				maxZ = std::max(maxZ, boundingSphere.position.z + boundingSphere.radius);
			}

			boundingSphere.position = MATHGL::Vector3{minX + maxX, minY + maxY, minZ + maxZ} / 2.0f;
			boundingSphere.radius = MATHGL::Vector3::Distance(boundingSphere.position, {minX, minY, minZ});
		}
	}
}

const std::vector<RESOURCES::Mesh*>& Model::getMeshes() const {
	return meshes;
}

const std::vector<std::string>& Model::getMaterialNames() const {
	return materialNames;
}


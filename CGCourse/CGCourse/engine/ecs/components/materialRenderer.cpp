#include "materialRenderer.h"

#include "modelRenderer.h"
#include "../../resourceManager/materialManager.h"

using namespace KUMA;
using namespace KUMA::ECS;

MaterialRenderer::MaterialRenderer(const ECS::Object & obj) : Component(obj) {
	__NAME__ = "MaterialRenderer";
	materials.fill(nullptr);
}

void MaterialRenderer::fillWithMaterial(std::shared_ptr<RENDER::Material> material) {
	for (auto i = 0u; i < materials.size(); i++) {
		materials[i] = material;
	}
}

void MaterialRenderer::setMaterial(unsigned index, std::shared_ptr<RENDER::Material> p_material) {
	materials[index] = p_material;
}

std::shared_ptr<RENDER::Material> MaterialRenderer::GetMaterialAtIndex(unsigned index) {
	return materials.at(index);
}

void MaterialRenderer::removeMaterial(unsigned index) {
	if (index < materials.size()) {
		materials[index] = nullptr;;
	}
}

void MaterialRenderer::removeMaterial(std::shared_ptr<RENDER::Material> p_instance) {
	for (uint8_t i = 0; i < materials.size(); i++) {
		if (materials[i] == p_instance) {
			materials[i] = nullptr;
		}
	}
}

void MaterialRenderer::removeMaterials() {
	for (auto i = 0u; i < materials.size(); i++) {
		materials[i] = nullptr;
	}
}

const MATHGL::Matrix4 & MaterialRenderer::getUserMatrix() const {
	return userMatrix;
}

const MaterialRenderer::MaterialList& MaterialRenderer::getMaterials() const {
	return materials;
}

void MaterialRenderer::setUserMatrixElement(unsigned row, unsigned col, float value) {
	if (row < 4 && col < 4) {
		userMatrix.data[4 * row + col] = value;
	}
}

float MaterialRenderer::getUserMatrixElement(unsigned  row, unsigned col) const {
	if (row < 4 && col < 4) {
		return userMatrix.data[4 * row + col];
	}
	return 0.0f;
}

void MaterialRenderer::onDeserialize(nlohmann::json& j) {
	for (auto& m : materials) {
		if (m)
			j["data"]["materials"].push_back(m->path);
	}
}
void MaterialRenderer::onSerialize(nlohmann::json& j) {
	int i = 0;
	for (auto& m : j["data"]["materials"]) {
		materials[i] = RESOURCES::MaterialLoader::Create(m);
		i++;
	}
}

#include "materialRenderer.h"

#include "modelRenderer.h"
#include "../../resourceManager/materialManager.h"

using namespace KUMA;
using namespace KUMA::ECS;

MaterialRenderer::MaterialRenderer(Ref<ECS::Object> obj) : Component(obj) {
	__NAME__ = "MaterialRenderer";
	materials.fill(nullptr);
}

void MaterialRenderer::fillWithMaterial(std::shared_ptr<RENDER::MaterialInterface> material) {
	for (auto i = 0u; i < materials.size(); i++) {
		materials[i] = material;
	}
}

void MaterialRenderer::setMaterial(unsigned index, std::shared_ptr<RENDER::MaterialInterface> p_material) {
	materials[index] = p_material;
}

std::shared_ptr<RENDER::MaterialInterface> MaterialRenderer::GetMaterialAtIndex(unsigned index) {
	return materials.at(index);
}

void MaterialRenderer::removeMaterial(unsigned index) {
	if (index < materials.size()) {
		materials[index] = nullptr;;
	}
}

void MaterialRenderer::removeMaterial(std::shared_ptr<RENDER::MaterialInterface> p_instance) {
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

const MaterialRenderer::MaterialList& MaterialRenderer::getMaterials() const {
	return materials;
}

void MaterialRenderer::onDeserialize(nlohmann::json& j) {
	int i = 0;
	for (auto& m : j["materials"]) {
		materials[i] = RESOURCES::MaterialLoader::Create(m);
		i++;
	}
	
}
void MaterialRenderer::onSerialize(nlohmann::json& j) {
	for (auto& m : materials) {
		if (m) {
			j["materials"].push_back(m->mPath);
		}
	}
}

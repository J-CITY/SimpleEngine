#include "materialRenderer.h"

#include "modelRenderer.h"
#include "transform.h"
#include "../../resourceManager/materialManager.h"

using namespace IKIGAI;
using namespace IKIGAI::ECS;

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

void MaterialRenderer::setMaterialsByPath(std::vector<std::string> paths) {
	int i = 0;
	for (auto& m : paths) {
		materials[i] = RESOURCES::MaterialLoader::Create(m);
		i++;
	}
}

std::vector<std::string> MaterialRenderer::getMaterialsPaths() {
	std::vector<std::string> res;
	for (auto& e : materials) {
		if (!e) {
			break;
		}
		res.push_back(e->mPath);
	}
	return res;
}


#include <rttr/registration>

RTTR_REGISTRATION
{
	rttr::registration::class_<IKIGAI::ECS::MaterialRenderer>("MaterialRenderer")
	(
		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE)
	)
	.property("Materials", &IKIGAI::ECS::MaterialRenderer::getMaterialsPaths, &IKIGAI::ECS::MaterialRenderer::setMaterialsByPath)
	(
		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE | MetaInfo::USE_IN_EDITOR_COMPONENT_INSPECTOR)
	)
	;
}
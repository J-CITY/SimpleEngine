#include "modelRenderer.h"

#include "materialRenderer.h"
#include "../object.h"
#include "../../resourceManager/modelManager.h"

using namespace IKIGAI;
using namespace IKIGAI::ECS;

ModelRenderer::ModelRenderer(Ref<ECS::Object> p_owner): Component(p_owner) {
	__NAME__ = "ModelRenderer";
}

void ModelRenderer::setModel(std::shared_ptr<RENDER::ModelInterface> model) {
	this->model = model;
}

std::shared_ptr<RENDER::ModelInterface> ModelRenderer::getModel() const {
	return model;
}

void ModelRenderer::setFrustumBehaviour(EFrustumBehaviour mode) {
	frustumBehaviour = mode;
}

ModelRenderer::EFrustumBehaviour ModelRenderer::getFrustumBehaviour() const {
	return frustumBehaviour;
}

const RENDER::BoundingSphere& ModelRenderer::getCustomBoundingSphere() const {
	return customBoundingSphere;
}

void ModelRenderer::setCustomBoundingSphere(const RENDER::BoundingSphere& boundingSphere) {
	customBoundingSphere = boundingSphere;
}

void ModelRenderer::setModelByPath(std::string path) {
	model = RESOURCES::ModelLoader().CreateFromFile(path);
	setFrustumBehaviour(IKIGAI::ECS::ModelRenderer::EFrustumBehaviour::CULL_MODEL);
	auto bs = IKIGAI::RENDER::BoundingSphere();
	bs.position = { 0.0f, 0.0f, 0.0f };
	bs.radius = 1.0f;
	setCustomBoundingSphere(bs);
}

std::string ModelRenderer::getModelPath() {
	if (!model) {
		return "";
	}
	return model->getPath();
}


#include <rttr/registration>

RTTR_REGISTRATION
{
	rttr::registration::class_<IKIGAI::ECS::ModelRenderer>("ModelRenderer")
	(
		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE)
	)
	.property("Model", &IKIGAI::ECS::ModelRenderer::getModelPath, &IKIGAI::ECS::ModelRenderer::setModelByPath)
	(
		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE | MetaInfo::USE_IN_EDITOR_COMPONENT_INSPECTOR),
		rttr::metadata(EditorMetaInfo::EDIT_WIDGET, EditorMetaInfo::WidgetType::STRING)
	);
}

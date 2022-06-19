#include "modelRenderer.h"

#include "materialRenderer.h"
#include "../object.h"
#include "../../resourceManager/modelManager.h"

using namespace KUMA;
using namespace KUMA::ECS;

ModelRenderer::ModelRenderer(const ECS::Object& p_owner): Component(p_owner) {
	__NAME__ = "ModelRenderer";
}

void ModelRenderer::setModel(std::shared_ptr<RENDER::Model> model) {
	this->model = model;
}

std::shared_ptr<RENDER::Model> ModelRenderer::getModel() const {
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

void ModelRenderer::onDeserialize(nlohmann::json& j) {
	MATHGL::Vector3 dump;
	RESOURCES::DeserializeVec3(j["data"]["customBoundingSphere"]["position"], dump);
	customBoundingSphere.position = dump;
	customBoundingSphere.radius = j["data"]["customBoundingSphere"]["radius"];
	frustumBehaviour = j["data"]["frustumBehaviour"];
	model = RESOURCES::ModelLoader().CreateFromFile(j["data"]["model"]["path"]);
}
void ModelRenderer::onSerialize(nlohmann::json& j) {
	RESOURCES::SerializeVec3(j["data"]["customBoundingSphere"]["position"], customBoundingSphere.position);
	j["data"]["customBoundingSphere"]["radius"] = customBoundingSphere.radius;
	if (model) {
		j["data"]["model"]["path"] = model->path;
	}
}

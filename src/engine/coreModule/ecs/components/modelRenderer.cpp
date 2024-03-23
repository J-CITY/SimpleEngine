#include "modelRenderer.h"

#include "materialRenderer.h"
#include "../object.h"
#include "resourceModule/modelManager.h"
#include "utilsModule/log/loggerDefine.h"

using namespace IKIGAI;
using namespace IKIGAI::ECS;

ModelRenderer::ModelRenderer(UTILS::Ref<ECS::Object> p_owner): Component(p_owner) {
	__NAME__ = "ModelRenderer";
}

ModelRenderer::ModelRenderer(UTILS::Ref<ECS::Object> _obj, const Descriptor& _descriptor): Component(_obj) {
	__NAME__ = "ModelRenderer";
	setModelByPath(_descriptor.Path);
}

void ModelRenderer::setModel(std::shared_ptr<RENDER::ModelInterface> model) {
	this->m_model = model;
	obj->setModelEvent.run();
}

std::shared_ptr<RENDER::ModelInterface> ModelRenderer::getModel() const {
	return m_model;
}

void ModelRenderer::setFrustumBehaviour(EFrustumBehaviour mode) {
	m_frustumBehaviour = mode;
}

EFrustumBehaviour ModelRenderer::getFrustumBehaviour() const {
	return m_frustumBehaviour;
}

const RENDER::BoundingSphere& ModelRenderer::getCustomBoundingSphere() const {
	return m_customBoundingSphere;
}

void ModelRenderer::setCustomBoundingSphere(const RENDER::BoundingSphere& boundingSphere) {
	m_customBoundingSphere = boundingSphere;
}

void ModelRenderer::setModelByPath(std::string path) {
	m_model = RESOURCES::ModelLoader().CreateFromFile(path);
	setFrustumBehaviour(IKIGAI::ECS::EFrustumBehaviour::CULL_MODEL);
	auto bs = IKIGAI::RENDER::BoundingSphere();
	bs.position = { 0.0f, 0.0f, 0.0f };
	bs.radius = 1.0f;
	setCustomBoundingSphere(bs);
	//obj->setModelEvent.run();
	//TODO:: remove this dependence
	auto mat = obj->getComponent<MaterialRenderer>();
	if (!mat) {
		return;
	}
	mat->updateMaterialList();
}

std::string ModelRenderer::getModelPath() {
	if (!m_model) {
		return "";
	}
	return m_model->getPath();
}

//---------------------------------------


ModelLODRenderer::ModelLODRenderer(UTILS::Ref<ECS::Object> p_owner) : Component(p_owner) {
	__NAME__ = "ModelLODRenderer";
}

ModelLODRenderer::ModelLODRenderer(UTILS::Ref<ECS::Object> _obj, const Descriptor _descriptor) : Component(_obj)
{
	__NAME__ = "ModelLODRenderer";
	setModelsByPath(_descriptor.Paths);
}

void ModelLODRenderer::setModel(const ModelLODRenderer::ModelLod& model) {
	m_models.push_back(model);
	obj->setModelEvent.run();
}

void ModelLODRenderer::setModels(const std::vector<ModelLODRenderer::ModelLod>& models) {
	m_models = models;
}

const std::vector<ModelLODRenderer::ModelLod>& ModelLODRenderer::getModels() const {
	return m_models;
}

std::shared_ptr<RENDER::ModelInterface> ModelLODRenderer::getModelByDistance(float distance) {
	for (const auto& model : m_models) {
		if (model.m_distance > distance) {
			return model.m_model;
		}
	}
	LOG_ERROR << ("Can not find lod");
	return nullptr;
}

void ModelLODRenderer::setFrustumBehaviour(EFrustumBehaviour mode) {
	m_frustumBehaviour = mode;
}

EFrustumBehaviour ModelLODRenderer::getFrustumBehaviour() const {
	return m_frustumBehaviour;
}

const RENDER::BoundingSphere& ModelLODRenderer::getCustomBoundingSphere() const {
	return m_customBoundingSphere;
}

void ModelLODRenderer::setCustomBoundingSphere(const RENDER::BoundingSphere& boundingSphere) {
	m_customBoundingSphere = boundingSphere;
}

void ModelLODRenderer::setModelsByPath(std::vector<ModelLODRenderer::ModelLodRefl> paths) {
	for (auto path : paths) {
		if (path.Path.empty()) {
			continue;
		}
		auto model = RESOURCES::ModelLoader().CreateFromFile(path.Path);
		setFrustumBehaviour(IKIGAI::ECS::EFrustumBehaviour::CULL_MODEL);
		auto bs = IKIGAI::RENDER::BoundingSphere();
		bs.position = { 0.0f, 0.0f, 0.0f };
		bs.radius = 1.0f;
		setCustomBoundingSphere(bs);
		//obj->setModelEvent.run();
		//TODO:: remove this dependence
		auto mat = obj->getComponent<MaterialRenderer>();
		if (!mat) {
			return;
		}
		mat->updateMaterialList();

		m_models.push_back({ path.Distance, model });
	}
}

std::vector<ModelLODRenderer::ModelLodRefl> ModelLODRenderer::getModelsPath() {
	std::vector<ModelLODRenderer::ModelLodRefl> res;
	for (auto e : m_models) {
		res.push_back({ e.m_distance, e.m_model->getPath() });
	}
	return res;
}


//#include <rttr/registration>
//
//RTTR_REGISTRATION
//{
//	rttr::registration::class_<IKIGAI::ECS::ModelRenderer>("ModelRenderer")
//	(
//		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE)
//	)
//	.property("Model", &IKIGAI::ECS::ModelRenderer::getModelPath, &IKIGAI::ECS::ModelRenderer::setModelByPath)
//	(
//		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE | MetaInfo::USE_IN_EDITOR_COMPONENT_INSPECTOR),
//		rttr::metadata(EditorMetaInfo::EDIT_WIDGET, EditorMetaInfo::WidgetType::STRING_WITH_FILE_CHOOSE),
//		rttr::metadata(EditorMetaInfo::FILE_EXTENSION, std::string(".fbx,.obj"))
//	);
//
//
//	rttr::registration::class_<IKIGAI::ECS::ModelLODRenderer::ModelLodRefl>("ModelLodRefl")
//	(
//		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE)
//	)
//	.property("Distance", &IKIGAI::ECS::ModelLODRenderer::ModelLodRefl::m_distance)
//	.property("Path", &IKIGAI::ECS::ModelLODRenderer::ModelLodRefl::m_path);
//
//
//	rttr::registration::class_<IKIGAI::ECS::ModelLODRenderer>("ModelLODRenderer")
//	(
//		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE)
//	)
//	.property("Models", &IKIGAI::ECS::ModelLODRenderer::getModelsPath, &IKIGAI::ECS::ModelLODRenderer::setModelsByPath)
//	(
//		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE | MetaInfo::USE_IN_EDITOR_COMPONENT_INSPECTOR),
//		rttr::metadata(EditorMetaInfo::EDIT_WIDGET, EditorMetaInfo::WidgetType::MODEL_LOD),
//		rttr::metadata(EditorMetaInfo::FILE_EXTENSION, std::string(".fbx,.obj"))
//	);
//}


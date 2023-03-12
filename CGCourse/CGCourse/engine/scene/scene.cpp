#include "scene.h"

#include <algorithm>


#include "../ecs/ComponentManager.h"
#include "../resourceManager/parser/assimpParser.h"
#include "../render/backends/interface/renderEnums.h"

namespace KUMA
{
	namespace ECS
	{
		class Skeletal;
	}
}

namespace KUMA
{
	namespace RENDER
	{
		enum class CullingOptions;
	}
}

using namespace KUMA;
using namespace KUMA::SCENE_SYSTEM;

void Scene::init() {
	//Create Default skybox
	
	//skyboxObject = std::make_unique<KUMA::ECS::Object>(KUMA::ObjectId<ECS::Object>(999), "Skybox", "");

	//skyboxTexture = KUMA::RESOURCES::TextureLoader::CreateSkybox("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\sb\\");
	//auto mat = skyboxObject->addComponent<KUMA::ECS::MaterialRenderer>();
	//auto skyboxMat = std::make_shared<KUMA::RENDER::Material>();
	//auto s = KUMA::RESOURCES::ShaderLoader::CreateFromFile("Shaders\\skybox1.glsl");
	//skyboxMat->setShader(s);
	//skyboxMat->setBackfaceCulling(true);
	//skyboxMat->setBackfaceCulling(true);
	//skyboxMat->setDepthTest(true);
	//skyboxMat->setDepthWriting(true);
	//skyboxMat->setColorWriting(true);
	//skyboxMat->setGPUInstances(1);
	//skyboxMat->getShader()->setUniformInt("gCubemapTexture", 0);
	//mat->fillWithMaterial(skyboxMat);
	
	//auto model = skyboxObject->addComponent<KUMA::ECS::ModelRenderer>();
	//KUMA::RESOURCES::ModelParserFlags flags = KUMA::RESOURCES::ModelParserFlags::TRIANGULATE;
	//auto m = KUMA::RESOURCES::ModelLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\Engine\\Models\\Cube.fbx", flags);
	//model->setModel(m);
	//model->setFrustumBehaviour(KUMA::ECS::ModelRenderer::EFrustumBehaviour::CULL_MODEL);
	//auto bs = KUMA::RENDER::BoundingSphere();
	//bs.position = {0.0f, 0.0f, 0.0f};
	//bs.radius = 1.0f;
	//model->setCustomBoundingSphere(bs);
	//skyboxObject->getTransform()->setLocalPosition({ 0.0f, 0.0f, 0.0f });
	//skyboxObject->getTransform()->setLocalRotation({0.0f, 0.0f, 0.0f, 1.0f});
	//skyboxObject->getTransform()->setLocalScale({20.0f, 20.0f, 20.0f});
	
}

Scene::~Scene() {
	objects.clear();
}

void Scene::go() {
	isExecute = true;

	for (auto& o : objects) {
		if (o->getIsActive()) {
			o->onEnable();
		}
	}
	for (auto& o : objects) {
		if (o->getIsActive()) {
			o->onStart();
		}
	}
}

bool Scene::getIsExecute() const {
	return isExecute;
}

void Scene::update(std::chrono::duration<double> dt) {
	for (auto& o : objects) {
		o->onUpdate(dt);
	}
}

void Scene::fixedUpdate(std::chrono::duration<double> dt) {
	for (auto& o : objects) {
		o->onFixedUpdate(dt);
	}
}

void Scene::lateUpdate(std::chrono::duration<double> dt) {
	for (auto& o : objects) {
		o->onLateUpdate(dt);
	}
}

std::shared_ptr<KUMA::ECS::Object> Scene::createObject() {
	return createObject("New Actor");
}

std::shared_ptr<KUMA::ECS::Object> Scene::_createObject(const std::string& name, const std::string& tag) {
	auto instance = std::make_shared<ECS::Object>(idGenerator.generateId(), name, tag);
	if (isExecute) {
		instance->setActive(true);
		if (instance->getIsActive()) {
			instance->onEnable();
			instance->onStart();
		}
	}
	return instance;
}

std::shared_ptr<KUMA::ECS::Object> Scene::createObject(ObjectId<ECS::Object> actorID, const std::string& name, const std::string& tag) {
	objects.push_back(std::make_shared<ECS::Object>(actorID, name, tag));
	auto& instance = objects.back();
	if (isExecute) {
		instance->setActive(true);
		if (instance->getIsActive()) {
			instance->onEnable();
			instance->onStart();
		}
	}
	return instance;
}


std::shared_ptr<KUMA::ECS::Object> Scene::createObject(const std::string& name, const std::string& tag) {
	objects.push_back(std::make_shared<ECS::Object>(idGenerator.generateId(), name, tag));
	auto& instance = objects.back();
	if (isExecute) {
		instance->setActive(true);
		if (instance->getIsActive()) {
			instance->onEnable();
			instance->onStart();
		}
	}
	return instance;
}

bool Scene::destroyObject(std::shared_ptr<ECS::Object> p_target) {
	auto found = std::ranges::find_if(objects.begin(), objects.end(), [p_target](std::shared_ptr<ECS::Object> element) {
		return element == p_target;
	});

	if (found != objects.end()) {
		objects.erase(found);
		return true;
	}
	else {
		return false;
	}
}

std::shared_ptr<KUMA::ECS::Object> Scene::findObjectByName(const std::string& p_name) {
	auto result = std::ranges::find_if(objects.begin(), objects.end(), [p_name](std::shared_ptr<ECS::Object>& element) {
		return element->getName() == p_name;
	});

	if (result != objects.end()) {
		return *result;
	} 
	else {
		return nullptr;
	}
}

std::shared_ptr<KUMA::ECS::Object> Scene::findObjectByTag(const std::string& p_tag) {
	auto result = std::ranges::find_if(objects.begin(), objects.end(), [p_tag](std::shared_ptr<ECS::Object>& element) {
		return (*element).getTag() == p_tag;
	});

	if (result != objects.end()) {
		return *result;
	}
	else {
		return nullptr;
	}
}

std::shared_ptr<KUMA::ECS::Object> Scene::findObjectByID(ObjectId<ECS::Object> p_id) {
	auto result = std::ranges::find_if(objects.begin(), objects.end(), [p_id](std::shared_ptr<ECS::Object>& element) {
		return element->getID() == p_id;
	});

	if (result != objects.end()) {
		return *result;
	}
	else {
		return nullptr;
	}
}

std::vector<std::shared_ptr<KUMA::ECS::Object>> Scene::findObjectsByName(const std::string& p_name) {
	std::vector<std::shared_ptr<KUMA::ECS::Object>> actors;
	for (const auto& actor : objects) {
		if (actor->getName() == p_name) {
			actors.push_back(actor);
		}
	}
	return actors;
}

std::vector<std::shared_ptr<KUMA::ECS::Object>> Scene::findObjectsByTag(const std::string& p_tag) {
	std::vector<std::shared_ptr<KUMA::ECS::Object>> actors;
	for (const auto& actor : objects) {
		if (actor->getTag() == p_tag) {
			actors.push_back(actor);
		}
	}
	return actors;
}

std::span<std::shared_ptr<KUMA::ECS::Object>> Scene::getObjects() {
	return objects;
}

//TODO: void Scene::setMainCamera(Entity id) {}


std::optional<KUMA::Ref<KUMA::ECS::CameraComponent>> Scene::findMainCamera() {
	for (auto& camera : *ECS::ComponentManager::getInstance()->getComponentArray<ECS::CameraComponent>()) {
		if (camera.obj->getIsActive()) {
			return camera;
		}
	}
	return std::nullopt;
}


std::vector<KUMA::RENDER::LightOGL> Scene::findLightData() {
	std::vector<RENDER::LightOGL> result;
	
	for (auto& light : *ECS::ComponentManager::getInstance()->getComponentArray<ECS::SpotLight>()) {
		if (light.obj->getIsActive()) {
			auto ldata = light.getData().generateOGLStruct();
			result.push_back(ldata);
		}
	}
	for (auto& light : *ECS::ComponentManager::getInstance()->getComponentArray<ECS::DirectionalLight>()) {
		if (light.obj->getIsActive()) {
			auto ldata = light.getData().generateOGLStruct();
			result.push_back(ldata);
		}
	}
	for (auto& light : *ECS::ComponentManager::getInstance()->getComponentArray<ECS::PointLight>()) {
		if (light.obj->getIsActive()) {
			auto ldata = light.getData().generateOGLStruct();
			result.push_back(ldata);
		}
	}
	for (auto& light : *ECS::ComponentManager::getInstance()->getComponentArray<ECS::AmbientLight>()) {
		if (light.obj->getIsActive()) {
			auto ldata = light.getData().generateOGLStruct();
			result.push_back(ldata);
		}
	}
	for (auto& light : *ECS::ComponentManager::getInstance()->getComponentArray<ECS::AmbientSphereLight>()) {
		if (light.obj->getIsActive()) {
			auto ldata = light.getData().generateOGLStruct();
			result.push_back(ldata);
		}
	}
	//for (auto light : ECS::ComponentManager::getInstance()->getComponentArray<ECS::SpotLight>()) {
	//	if (light.second->obj.getIsActive()) {
	//		auto ldata = light.second->getData().generateOGLStruct();
	//		result.push_back(ldata);
	//	}
	//}
	return result;
}

std::vector<KUMA::RENDER::LightOGL> Scene::findLightDataInFrustum(const RENDER::Frustum& p_frustum) {
	std::vector<RENDER::LightOGL> result;
	
	for (auto& light : *ECS::ComponentManager::getInstance()->getComponentArray<ECS::SpotLight>()) {
		if (light.obj->getIsActive()) {
			const auto& lightData = light.getData();
			const auto& position = lightData.getTransform().getWorldPosition();
			auto effectRange = lightData.getEffectRange();

			if (std::isinf(effectRange) || p_frustum.sphereInFrustum(position.x, position.y, position.z, lightData.getEffectRange())) {
				auto ldata = lightData.generateOGLStruct();
				result.push_back(ldata);
			}
		}
	}
	for (auto& light : *ECS::ComponentManager::getInstance()->getComponentArray<ECS::DirectionalLight>()) {
		if (light.obj->getIsActive()) {
			const auto& lightData = light.getData();
			const auto& position = lightData.getTransform().getWorldPosition();
			auto effectRange = lightData.getEffectRange();

			if (std::isinf(effectRange) || p_frustum.sphereInFrustum(position.x, position.y, position.z, lightData.getEffectRange())) {
				auto ldata = lightData.generateOGLStruct();
				result.push_back(ldata);
			}
		}
	}
	for (auto& light : *ECS::ComponentManager::getInstance()->getComponentArray<ECS::PointLight>()) {
		if (light.obj->getIsActive()) {
			const auto& lightData = light.getData();
			const auto& position = lightData.getTransform().getWorldPosition();
			auto effectRange = lightData.getEffectRange();

			if (std::isinf(effectRange) || p_frustum.sphereInFrustum(position.x, position.y, position.z, lightData.getEffectRange())) {
				auto ldata = lightData.generateOGLStruct();
				result.push_back(ldata);
			}
		}
	}
	for (auto& light : *ECS::ComponentManager::getInstance()->getComponentArray<ECS::AmbientLight>()) {
		if (light.obj->getIsActive()) {
			const auto& lightData = light.getData();
			const auto& position = lightData.getTransform().getWorldPosition();
			auto effectRange = lightData.getEffectRange();

			if (std::isinf(effectRange) || p_frustum.sphereInFrustum(position.x, position.y, position.z, lightData.getEffectRange())) {
				auto ldata = lightData.generateOGLStruct();
				result.push_back(ldata);
			}
		}
	}
	for (auto& light : *ECS::ComponentManager::getInstance()->getComponentArray<ECS::AmbientSphereLight>()) {
		if (light.obj->getIsActive()) {
			const auto& lightData = light.getData();
			const auto& position = lightData.getTransform().getWorldPosition();
			auto effectRange = lightData.getEffectRange();

			if (std::isinf(effectRange) || p_frustum.sphereInFrustum(position.x, position.y, position.z, lightData.getEffectRange())) {
				auto ldata = lightData.generateOGLStruct();
				result.push_back(ldata);
			}
		}
	}
	//for (auto& light : ECS::ComponentManager::getInstance()->lightComponents) {
	//	if (light.second->obj.getIsActive()) {
	//		const auto& lightData = light.second->getData();
	//		const auto& position = lightData.getTransform().getWorldPosition();
	//		auto effectRange = lightData.getEffectRange();
	//		
	//		if (std::isinf(effectRange) || p_frustum.sphereInFrustum(position.x, position.y, position.z, lightData.getEffectRange())) {
	//			auto ldata = lightData.generateOGLStruct();
	//			result.push_back(ldata);
	//		}
	//	}
	//}
	return result;
}

//KUMA::ECS::Object& Scene::getSkybox() const {
//	return *skyboxObject;
//}
//
//KUMA::RESOURCES::CubeMap& Scene::getSkyboxTexture() const {
//	return *skyboxTexture;
//}

std::vector<std::shared_ptr<RENDER::MeshInterface>> Scene::getMeshesInFrustum(
	const RENDER::ModelInterface& model,
	const RENDER::BoundingSphere& modelBoundingSphere,
	const ECS::Transform& modelTransform,
	const RENDER::Frustum& frustum,
	RENDER::CullingOptions cullingOptions
) {
	const bool frustumPerModel = isFlagSet(RENDER::CullingOptions::FRUSTUM_PER_MODEL, cullingOptions);

	if (!frustumPerModel || frustum.boundingSphereInFrustum(modelBoundingSphere, modelTransform)) {
		std::vector<std::shared_ptr<RENDER::MeshInterface>> result;

		const bool frustumPerMesh = isFlagSet(RENDER::CullingOptions::FRUSTUM_PER_MESH, cullingOptions);

		auto& meshes = model.getMeshes();

		for (auto mesh : meshes) {
			if (meshes.size() == 1 || !frustumPerMesh || frustum.boundingSphereInFrustum(mesh->getBoundingSphere(), modelTransform)) {
				result.emplace_back(mesh);
			}
		}
		return result;
	}
	return {};
}


std::tuple<KUMA::RENDER::OpaqueDrawables,
KUMA::RENDER::TransparentDrawables,
KUMA::RENDER::OpaqueDrawables,
KUMA::RENDER::TransparentDrawables> Scene::findAndSortFrustumCulledDrawables
(
	const MATHGL::Vector3& cameraPosition,
	const RENDER::Frustum& frustum,
	std::shared_ptr<RENDER::MaterialInterface> defaultMaterial
) {
	RENDER::OpaqueDrawables opaqueDrawablesForward;
	RENDER::TransparentDrawables transparentDrawablesForward;
	RENDER::OpaqueDrawables opaqueDrawablesDeferred;
	RENDER::TransparentDrawables transparentDrawablesDeferred;

	for (const auto& modelRenderer : *ECS::ComponentManager::getInstance()->getComponentArray<ECS::ModelRenderer>()) {
		auto owner = modelRenderer.obj;

		if (owner->getIsActive()) {
			if (auto model = modelRenderer.getModel()) {
				if (auto materialRenderer = modelRenderer.obj.get().getComponent<ECS::MaterialRenderer>()) {
					auto& transform = owner->getTransform()->getTransform();
					auto animator = modelRenderer.obj.get().getComponent<ECS::Skeletal>();

					RENDER::CullingOptions cullingOptions = RENDER::CullingOptions::NONE;

					if (modelRenderer.getFrustumBehaviour() != ECS::ModelRenderer::EFrustumBehaviour::DISABLED) {
						cullingOptions |= RENDER::CullingOptions::FRUSTUM_PER_MODEL;
					}

					if (modelRenderer.getFrustumBehaviour() == ECS::ModelRenderer::EFrustumBehaviour::CULL_MESHES) {
						cullingOptions |= RENDER::CullingOptions::FRUSTUM_PER_MESH;
					}

					const auto& modelBoundingSphere = modelRenderer.getFrustumBehaviour() == ECS::ModelRenderer::EFrustumBehaviour::CULL_CUSTOM ? modelRenderer.getCustomBoundingSphere() : model->getBoundingSphere();
					
					auto meshes = getMeshesInFrustum(*model, modelBoundingSphere, transform, frustum, cullingOptions);

					if (!meshes.empty()) {
						float distanceToActor = MATHGL::Vector3::Distance(transform.getWorldPosition(), cameraPosition);
						const ECS::MaterialRenderer::MaterialList& materials = materialRenderer.value()->getMaterials();

						for (const auto mesh : meshes) {
							std::shared_ptr<RENDER::MaterialInterface> material;
							if (mesh->getMaterialIndex() < MAX_MATERIAL_COUNT) {
								material = materials.at(mesh->getMaterialIndex());
								if (!material || (!material->getShader() && !material->isDeferred())) {
									material = defaultMaterial;
								}
							}

							if (material) {
								RENDER::Drawable element = { transform.getPrevWorldMatrix(), transform.getWorldMatrix(), mesh, material, animator};
								transform.setPrevWorldMatrix(transform.getWorldMatrix());
								if (material->isBlendable()) {
									if (material->getShader()) {
										transparentDrawablesForward.emplace(distanceToActor, element);
									}
									else {
										transparentDrawablesDeferred.emplace(distanceToActor, element);
									}
								}
								else {
									if (material->getShader()) {
										opaqueDrawablesForward.emplace(distanceToActor, element);
									}
									else {
										opaqueDrawablesDeferred.emplace(distanceToActor, element);
									}
								}
							}
						}
					}
				}
			}
		}
	}

	return {opaqueDrawablesForward, transparentDrawablesForward, opaqueDrawablesDeferred, transparentDrawablesDeferred};
}

std::tuple<KUMA::RENDER::OpaqueDrawables,
	KUMA::RENDER::TransparentDrawables,
	KUMA::RENDER::OpaqueDrawables,
	KUMA::RENDER::TransparentDrawables>  Scene::findAndSortDrawables
(
	const MATHGL::Vector3& cameraPosition,
	std::shared_ptr<RENDER::MaterialInterface> defaultMaterial
) {
	RENDER::OpaqueDrawables opaqueDrawablesForward;
	RENDER::TransparentDrawables transparentDrawablesForward;
	RENDER::OpaqueDrawables opaqueDrawablesDeferred;
	RENDER::TransparentDrawables transparentDrawablesDeferred;

	for (auto& modelRenderer : *ECS::ComponentManager::getInstance()->getComponentArray<ECS::ModelRenderer>()) {
		if (modelRenderer.obj->getIsActive() && modelRenderer.obj->getName() != "Skybox") {
			if (auto model = modelRenderer.getModel()) {
				float distanceToActor = MATHGL::Vector3::Distance(modelRenderer.obj->getTransform()->getWorldPosition(), cameraPosition);

				if (auto materialRenderer = modelRenderer.obj->getComponent<ECS::MaterialRenderer>()) {
					auto& transform = modelRenderer.obj->getTransform()->getTransform();
					auto animator = modelRenderer.obj->getComponent<ECS::Skeletal>();

					const ECS::MaterialRenderer::MaterialList& materials = materialRenderer.value()->getMaterials();

					for (const auto mesh : model->getMeshes()) {
						std::shared_ptr<RENDER::MaterialInterface> material;
						if (mesh->getMaterialIndex() < MAX_MATERIAL_COUNT) {
							material = materials.at(mesh->getMaterialIndex());
							if (!material || (!material->getShader() && !material->isDeferred())) {
								material = defaultMaterial;
							}
						}

						if (material) {
							RENDER::Drawable element = { transform.getPrevWorldMatrix(),transform.getWorldMatrix(), mesh, material, animator};
							transform.setPrevWorldMatrix(transform.getWorldMatrix());
							if (material->isBlendable()) {
								if (!material->isDeferred()) {
									transparentDrawablesForward.emplace(distanceToActor, element);
								}
								else {
									transparentDrawablesDeferred.emplace(distanceToActor, element);
								}
							}
							else {
								if (!material->isDeferred()) {
									opaqueDrawablesForward.emplace(distanceToActor, element);
								}
								else {
									opaqueDrawablesDeferred.emplace(distanceToActor, element);
								}
							}
						}
					}
				}
			}
		}
	}

	return {opaqueDrawablesForward, transparentDrawablesForward, opaqueDrawablesDeferred, transparentDrawablesDeferred};
}


std::tuple<KUMA::RENDER::OpaqueDrawables,
	KUMA::RENDER::TransparentDrawables,
	KUMA::RENDER::OpaqueDrawables,
	KUMA::RENDER::TransparentDrawables> Scene::findDrawables(
	const MATHGL::Vector3& cameraPosition,
	const RENDER::Camera& camera,
	const RENDER::Frustum* customFrustum,
	std::shared_ptr<RENDER::MaterialInterface> defaultMaterial
) {
	if (camera.isFrustumGeometryCulling()) {
		const auto& frustum = customFrustum ? *customFrustum : camera.getFrustum();
		return findAndSortFrustumCulledDrawables(cameraPosition, frustum, defaultMaterial);
	}
	else {
		return findAndSortDrawables(cameraPosition, defaultMaterial);
	}
}

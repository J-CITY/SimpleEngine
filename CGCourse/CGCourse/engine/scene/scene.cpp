#include "scene.h"

#include <algorithm>


#include "../ecs/ComponentManager.h"
#include "../ecs/components/inputComponent.h"
#include "../resourceManager/modelManager.h"
#include "../resourceManager/shaderManager.h"
#include "../resourceManager/parser/assimpParser.h"

using namespace KUMA::SCENE_SYSTEM;

void Scene::init() {
	//Create Default skybox
	
	skyboxObject = std::make_unique<KUMA::ECS::Object>(KUMA::ObjectId<ECS::Object>(-1), "Skybox", "");

	skyboxTexture = KUMA::RESOURCES::TextureLoader::CreateSkybox("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\sb\\");
	auto mat = skyboxObject->addComponent<KUMA::ECS::MaterialRenderer>();
	auto skyboxMat = std::make_shared<KUMA::RENDER::Material>();
	auto s = KUMA::RESOURCES::ShaderLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\skybox.glsl");
	skyboxMat->setShader(s);
	skyboxMat->setBackfaceCulling(true);
	skyboxMat->setBackfaceCulling(true);
	skyboxMat->setDepthTest(true);
	skyboxMat->setDepthWriting(true);
	skyboxMat->setColorWriting(true);
	skyboxMat->setGPUInstances(1);
	skyboxMat->getShader()->setUniformInt("gCubemapTexture", 0);
	mat->fillWithMaterial(skyboxMat);
	
	auto model = skyboxObject->addComponent<KUMA::ECS::ModelRenderer>();
	KUMA::RESOURCES::ModelParserFlags flags = KUMA::RESOURCES::ModelParserFlags::TRIANGULATE;
	auto m = KUMA::RESOURCES::ModelLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\Engine\\Models\\Cube.fbx", flags);
	model->setModel(m);
	model->setFrustumBehaviour(KUMA::ECS::ModelRenderer::EFrustumBehaviour::CULL_MODEL);
	auto bs = KUMA::RENDER::BoundingSphere();
	bs.position = {0.0f, 0.0f, 0.0f};
	bs.radius = 1.0f;
	model->setCustomBoundingSphere(bs);
	skyboxObject->transform->setLocalPosition({0.0f, 0.0f, 0.0f});
	skyboxObject->transform->setLocalRotation({0.0f, 0.0f, 0.0f, 1.0f});
	skyboxObject->transform->setLocalScale({20.0f, 20.0f, 20.0f});
	
}

Scene::~Scene() {
	objects.clear();
}

void Scene::go() {
	isExecute = true;

	for (auto& o : objects) {
		//o->setActive(true);
	}
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

void Scene::update(float dt) {
	for (auto& o : objects) {
		o->onUpdate(dt);
	}
}

void Scene::fixedUpdate(float dt) {
	for (auto& o : objects) {
		o->onFixedUpdate(dt);
	}
}

void Scene::lateUpdate(float dt) {
	for (auto& o : objects) {
		o->onLateUpdate(dt);
	}
}

KUMA::ECS::Object& Scene::createObject() {
	return createObject("New Actor");
}

KUMA::ECS::Object& Scene::createObject(const std::string& name, const std::string& tag) {
	objects.push_back(std::make_shared<ECS::Object>(idGenerator.generateId(), name, tag));
	auto& instance = *objects.back();
	instance.componentAddedEvent.add(std::bind(&Scene::onComponentAdded, this, std::placeholders::_1));
	instance.componentRemovedEvent.add(std::bind(&Scene::onComponentRemoved, this, std::placeholders::_1));
	if (isExecute) {
		instance.setActive(true);
		if (instance.getIsActive()) {
			instance.onEnable();
			instance.onStart();
		}
	}
	return instance;
}

bool Scene::destroyActor(ECS::Object& p_target) {
	auto found = std::find_if(objects.begin(), objects.end(), [&p_target](std::shared_ptr<ECS::Object>& element) {
		return element.get() == &p_target;
	});

	if (found != objects.end()) {
		objects.erase(found);
		return true;
	}
	else {
		return false;
	}
}

KUMA::ECS::Object* Scene::findActorByName(const std::string& p_name) {
	auto result = std::find_if(objects.begin(), objects.end(), [p_name](std::shared_ptr<ECS::Object>& element) {
		return element->getName() == p_name;
	});

	if (result != objects.end())
		return (*result).get();
	else
		return nullptr;
}

KUMA::ECS::Object* Scene::findActorByTag(const std::string& p_tag) {
	auto result = std::find_if(objects.begin(), objects.end(), [p_tag](std::shared_ptr<ECS::Object>& element) {
		return (*element).getTag() == p_tag;
	});

	if (result != objects.end())
		return (*result).get();
	else
		return nullptr;
}

std::shared_ptr<KUMA::ECS::Object> Scene::findActorByID(ObjectId<ECS::Object> p_id) {
	auto result = std::find_if(objects.begin(), objects.end(), [p_id](std::shared_ptr<ECS::Object>& element) {
		return element->getID() == p_id;
	});

	if (result != objects.end())
		return *result;
	else
		return nullptr;
}

std::vector<std::reference_wrapper<KUMA::ECS::Object>> Scene::findActorsByName(const std::string& p_name) {
	std::vector<std::reference_wrapper<KUMA::ECS::Object>> actors;

	for (auto actor : objects) {
		if (actor->getName() == p_name)
			actors.push_back(std::ref(*actor));
	}

	return actors;
}

std::vector<std::reference_wrapper<KUMA::ECS::Object>> Scene::findActorsByTag(const std::string& p_tag) {
	std::vector<std::reference_wrapper<KUMA::ECS::Object>> actors;

	for (auto actor : objects) {
		if (actor->getTag() == p_tag)
			actors.push_back(std::ref(*actor));
	}

	return actors;
}

void Scene::onComponentAdded(std::shared_ptr<ECS::Component> p_compononent) {
	if (auto result = std::dynamic_pointer_cast<ECS::ModelRenderer>(p_compononent))
		ECS::ComponentManager::getInstance()->addComponent(result->obj.getID(), result);
	else if (auto result = std::dynamic_pointer_cast<ECS::CameraComponent>(p_compononent))
		ECS::ComponentManager::getInstance()->addComponent(result->obj.getID(), result);
	else if (auto result = std::dynamic_pointer_cast<ECS::LightComponent>(p_compononent)) {
		ECS::ComponentManager::getInstance()->addComponent(result->obj.getID(), result);

	}
	else if (auto result = std::dynamic_pointer_cast<ECS::InputComponent>(p_compononent))
		ECS::ComponentManager::getInstance()->addComponent(result->obj.getID(), result);
	else if (auto result = std::dynamic_pointer_cast<ECS::MaterialRenderer>(p_compononent))
		ECS::ComponentManager::getInstance()->addComponent(result->obj.getID(), result);
	else if (auto result = std::dynamic_pointer_cast<ECS::Script>(p_compononent))
		ECS::ComponentManager::getInstance()->addComponent(result->obj.getID(), result);
}

void Scene::onComponentRemoved(std::shared_ptr<ECS::Component> p_compononent) {
	if (auto result = std::dynamic_pointer_cast<ECS::ModelRenderer>(p_compononent))
		ECS::ComponentManager::getInstance()->modelComponents.erase(p_compononent->obj.getID());

	else if (auto result = std::dynamic_pointer_cast<ECS::CameraComponent>(p_compononent))
		ECS::ComponentManager::getInstance()->cameraComponents.erase(p_compononent->obj.getID());

	else if (auto result = std::dynamic_pointer_cast<ECS::LightComponent>(p_compononent))
		ECS::ComponentManager::getInstance()->lightComponents.erase(p_compononent->obj.getID());

	//TODO: add other components
}

std::vector<std::shared_ptr<KUMA::ECS::Object>>& Scene::getObjects() {
	return objects;
}

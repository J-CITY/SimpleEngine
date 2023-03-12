#include "sceneManager.h"

#include "../ecs/components/ambientSphereLight.h"
#include "../ecs/components/cameraComponent.h"
#include "../ecs/components/directionalLight.h"

using namespace KUMA;
using namespace KUMA::SCENE_SYSTEM;

SceneManager::SceneManager(const std::string& p_sceneRootFolder) : m_sceneRootFolder(p_sceneRootFolder) {
	loadEmptyScene();
}

SceneManager::~SceneManager() {
	unloadCurrentScene();
}

void SceneManager::update() {
	if (m_delayedLoadCall) {
		m_delayedLoadCall();
		m_delayedLoadCall = 0;
	}
}

void SceneManager::loadEmptyScene() {
	unloadCurrentScene();

	m_currentScene = std::make_unique<Scene>();

	sceneLoadEvent.run();
}

void SceneManager::loadDefaultScene() {
	unloadCurrentScene();

	m_currentScene = std::make_unique<Scene>();

	sceneLoadEvent.run();

	auto directionalLight = m_currentScene->createObject("Directional Light");
	directionalLight->addComponent<ECS::DirectionalLight>()->setIntensity(0.75f);
	directionalLight->getTransform()->setLocalPosition({0.0f, 10.0f, 0.0f});
	directionalLight->getTransform()->setLocalRotation(MATHGL::Quaternion(MATHGL::Vector3{120.0f, -40.0f, 0.0f}));

	auto ambientLight = m_currentScene->createObject("Ambient Light");
	ambientLight->addComponent<ECS::AmbientSphereLight>()->setRadius(10000.0f);

	auto camera = m_currentScene->createObject("Main Camera");
	camera->addComponent<ECS::CameraComponent>();
	camera->getTransform()->setLocalRotation(MATHGL::Quaternion(MATHGL::Vector3{ 20.0f, 180.0f, 0.0f }));
	camera->getTransform()->setLocalPosition({0.0f, 3.0f, 8.0f});
}

#include <fstream>
#include <json.hpp>
#include "../utils/loader.h"
void SceneManager::loadFromFile(const std::string& sceneFilePath) {
	unloadCurrentScene();

	m_currentScene = std::make_unique<Scene>();

	std::ifstream f(UTILS::getRealPath(sceneFilePath));
	nlohmann::json data = nlohmann::json::parse(f);
	f.close();
	if (data.contains("actors")) {
		std::string name;
		std::string tag;
		ObjectId<ECS::Object> id(0u);
		for (auto& actor : data["actors"]) {
			if (actor.contains("name")) {
				name = actor["name"].get<std::string>();
			}
			if (actor.contains("tag")) {
				tag = actor["tag"].get<std::string>();
			}
			if (actor.contains("id")) {
				id = ObjectId<ECS::Object>(actor["id"].get<unsigned>());
			}
			auto obj = m_currentScene->createObject(id, name, tag);

			if (actor.contains("components")) {
				for (auto& component: actor["components"]) {
					std::string type = component["type"];
					auto n = typeid(ECS::Transform).name();
					if (type == typeid(ECS::TransformComponent).name()) {
						
						auto pos = RESOURCES::DeserializeVec3(component["pos"]);
						auto scale = RESOURCES::DeserializeVec3(component["scale"]);
						auto rotate = RESOURCES::DeserializeQuat(component["rotate"]);
						obj->getTransform()->getTransform().generateMatrices(pos, rotate, scale);
					}
					else if (type == typeid(ECS::CameraComponent).name()) {
						auto c = obj->addComponent<KUMA::ECS::CameraComponent>();
						c.getPtr()->onDeserialize(component);
					}
					else if (type == typeid(ECS::DirectionalLight).name()) {
						auto c = obj->addComponent<KUMA::ECS::DirectionalLight>();
						c.getPtr()->onDeserialize(component);
					}
					else if (type == typeid(ECS::AmbientSphereLight).name()) {
						auto c = obj->addComponent<KUMA::ECS::AmbientSphereLight>();
						c.getPtr()->onDeserialize(component);
					}
					else if (type == typeid(ECS::ModelRenderer).name()) {
						auto c = obj->addComponent<KUMA::ECS::ModelRenderer>();
						c.getPtr()->onDeserialize(component);
					}
					else if (type == typeid(ECS::MaterialRenderer).name()) {
						auto c = obj->addComponent<KUMA::ECS::MaterialRenderer>();
						c.getPtr()->onDeserialize(component);
					}
					else if (type == typeid(ECS::ScriptComponent).name()) {
						auto c = obj->addComponent<KUMA::ECS::ScriptComponent>("Controller");
						c.getPtr()->onDeserialize(component);
					}
					
				}
			}
		}
	}

}


void SceneManager::unloadCurrentScene() {
	if (m_currentScene) {
		m_currentScene.release();
		m_currentScene = nullptr;
		sceneUnloadEvent.run();
	}
}

bool SceneManager::hasCurrentScene() const {
	return m_currentScene != nullptr;
}

Scene& SceneManager::getCurrentScene() {
	return *m_currentScene;
}

std::string SceneManager::getCurrentSceneSourcePath() const {
	return m_currentSceneSourcePath;
}


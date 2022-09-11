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
	directionalLight->transform->setLocalPosition({0.0f, 10.0f, 0.0f});
	directionalLight->transform->setLocalRotation(MATHGL::Quaternion(MATHGL::Vector3{120.0f, -40.0f, 0.0f}));

	auto ambientLight = m_currentScene->createObject("Ambient Light");
	ambientLight->addComponent<ECS::AmbientSphereLight>()->setRadius(10000.0f);

	auto camera = m_currentScene->createObject("Main Camera");
	camera->addComponent<ECS::CameraComponent>();
	camera->transform->setLocalPosition({0.0f, 3.0f, 8.0f});
	camera->transform->setLocalRotation(MATHGL::Quaternion(MATHGL::Vector3{20.0f, 180.0f, 0.0f}));
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


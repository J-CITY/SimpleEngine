#include "sceneManager.h"

#include <coreModule/ecs/components/ambientSphereLight.h>
#include <coreModule/ecs/components/cameraComponent.h>
#include <coreModule/ecs/components/directionalLight.h>
#include <coreModule/ecs/object.h>

#include "utilsModule/jsonParser/jsonParser.h"

import logger;

using namespace IKIGAI;
using namespace IKIGAI::SCENE_SYSTEM;

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

	m_currentScene->postLoad();
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

	m_currentScene->postLoad();
}

#include <fstream>
#include <nlohmann/json.hpp>
#include <utilsModule/loader.h>

template<typename T>
void addComponentImpl(const std::string& typeNameStr, std::shared_ptr<ECS::Object> obj, nlohmann::json& data) {
	if (typeNameStr == typeid(T).name()) {
		auto c = obj->addComponent<T>();
		auto result = IKIGAI::UTILS::FromJson<T>(*c, data);
		if (result.isErr()) {
			LOG_ERROR(result.unwrapErr().text);
		}
	}
}

template<template<typename...> class Container, typename...ComponentType>
void addComponent(const std::string& typeNameStr, std::shared_ptr<ECS::Object> obj, nlohmann::json& data, Container<ComponentType...> opt) {
	(addComponentImpl<ComponentType>(typeNameStr, obj, data), ...);
}

void addComponent(const std::string& typeNameStr, std::shared_ptr<ECS::Object> obj, nlohmann::json& data) {
	addComponent(typeNameStr, obj, data, ECS::ComponentsTypeProviderType{});
}

//----------------------------

template<typename T>
void saveComponentImpl(const std::string& typeNameStr, UTILS::WeakPtr<ECS::Component> component, nlohmann::json& data) {
	if (typeNameStr == typeid(T).name()) {
		auto _p = std::static_pointer_cast<T>(component);
		data = IKIGAI::UTILS::ToJson(*_p);
		data["Type"] = typeid(T).name();
	}
}

template<template<typename...> class Container, typename...ComponentType>
void saveComponent(const std::string& typeNameStr, UTILS::WeakPtr<ECS::Component> component, nlohmann::json& data, Container<ComponentType...> opt) {
	(saveComponentImpl<ComponentType>(typeNameStr, component, data), ...);
}

void saveComponent(UTILS::WeakPtr<ECS::Component> component, nlohmann::json& data) {
	saveComponent(component->getTypeidName(), component, data, ECS::ComponentsTypeProviderType{});
}

void SceneManager::loadFromFile(const std::string& sceneFilePath) {
	unloadCurrentScene();

	m_currentScene = std::make_unique<Scene>();
	m_currentSceneSourcePath = sceneFilePath;

	std::ifstream f(UTILS::getRealPath(sceneFilePath));
	nlohmann::json data = nlohmann::json::parse(f);
	f.close();
	if (data.contains("Actors")) {
		for (auto& actor : data["Actors"]) {
			auto newActorRes = IKIGAI::UTILS::FromJson<ECS::ObjectData>(actor);
			if (newActorRes.isErr()) {
				auto err = newActorRes.unwrapErr();
				LOG_ERROR(err.text);
				continue;
			}
			auto newActor = newActorRes.unwrap();
			auto obj = m_currentScene->createObject(newActor);
			if (actor.contains("Components")) {
				for (auto& component: actor["Components"]) {
					addComponent(component["Type"], obj, component);
				}
			}
		}
	}

	m_currentScene->postLoad();
}

void SceneManager::saveToFile() {
	saveToFile(m_currentSceneSourcePath);
}

void SceneManager::saveToFile(const std::string& sceneFilePath) {
	nlohmann::json data;

	data["Actors"] = std::vector<nlohmann::json>();
	for (auto& actor : m_currentScene->getObjects()) {
		auto actorData = actor->getObjectData();
		auto actorJson = IKIGAI::UTILS::ToJson(actorData);
		actorJson["Components"] = std::vector<nlohmann::json>();
		
		for (auto& component : actor->getComponents()) {
			actorJson["Components"].push_back({});
			saveComponent(component, actorJson["Components"].back());
		}

		data["Actors"].push_back(actorJson);
	}

	//write
	std::ofstream f(UTILS::getRealPath(sceneFilePath));
	f << data.dump(4) << std::endl;
	f.close();
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

void SceneManager::setCurrentSceneSourcePath(const std::string& name) {
	m_currentSceneSourcePath = "Scenes/" + name + ".xml";
}


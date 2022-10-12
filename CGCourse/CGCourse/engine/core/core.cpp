#include "core.h"

#include "../config.h"
#include "../render/gameRenderer.h"
#include "../resourceManager/materialManager.h"
#include "../resourceManager/modelManager.h"
#include "../resourceManager/ServiceManager.h"
#include "../resourceManager/shaderManager.h"
#include "../resourceManager/textureManager.h"
#include "../audioManager/audioManager.h"
#include "../debug/debugRender.h"
#include "../physics/PhysicWorld.h"
#include "../tasks/taskSystem.h"
#include "../utils/loader.h"
#include "../glManager/glManager.h"
#include "../inputManager/inputManager.h"
#include "../render/render.h"
#include "../scene/sceneManager.h"
#include "../debug/debugRender.h"

import logger;

namespace KUMA
{
	namespace ECS
	{
		class InputComponent;
		class LogicComponent;
		class AmbientSphereLight;
		class AmbientLight;
	}
}

using namespace KUMA;
using namespace KUMA::CORE_SYSTEM;

Core::Core() {
	RESOURCES::ModelLoader::SetAssetPaths(Config::USER_ASSETS_PATH, Config::ENGINE_ASSETS_PATH);
	RESOURCES::TextureLoader::SetAssetPaths(Config::USER_ASSETS_PATH, Config::ENGINE_ASSETS_PATH);
	RESOURCES::ShaderLoader::SetAssetPaths(Config::USER_ASSETS_PATH, Config::ENGINE_ASSETS_PATH);
	RESOURCES::MaterialLoader::SetAssetPaths(Config::USER_ASSETS_PATH, Config::ENGINE_ASSETS_PATH);

	auto windowSettings = KUMA::UTILS::loadConfigFile<WINDOW_SYSTEM::WindowSettings>("Configs\\window.json");
	if (windowSettings.isErr()) {
		LOG_ERROR(windowSettings.unwrapErr().msg);
		throw;
	}
	window = std::make_unique<WINDOW_SYSTEM::Window>(windowSettings.unwrap());
	sceneManager = std::make_unique<SCENE_SYSTEM::SceneManager>(Config::ENGINE_ASSETS_PATH);
	inputManager = std::make_unique<INPUT_SYSTEM::InputManager>(*window);
	driver = std::make_unique<GL_SYSTEM::GlManager>(GL_SYSTEM::DriverSettings{false});
	scriptInterpreter = std::make_unique<SCRIPTING::ScriptInterpreter>(Config::ROOT + Config::USER_ASSETS_PATH + "scripts\\");
	audioManager = std::make_unique<AUDIO::AudioManager>();
	physicsManger = std::make_unique<PHYSICS::PhysicWorld>(256);
	taskManger = std::make_unique<TASK::TaskSystem>();
	taskManger->setup();

	ECS::ComponentManager::getInstance()->registerComponent<ECS::TransformComponent>();
	ECS::ComponentManager::getInstance()->registerComponent<ECS::AmbientLight>();
	ECS::ComponentManager::getInstance()->registerComponent<ECS::AmbientSphereLight>();
	ECS::ComponentManager::getInstance()->registerComponent<ECS::AudioComponent>();
	ECS::ComponentManager::getInstance()->registerComponent<ECS::CameraComponent>();
	ECS::ComponentManager::getInstance()->registerComponent<ECS::DirectionalLight>();
	ECS::ComponentManager::getInstance()->registerComponent<ECS::InputComponent>();
	ECS::ComponentManager::getInstance()->registerComponent<ECS::LogicComponent>();
	ECS::ComponentManager::getInstance()->registerComponent<ECS::MaterialRenderer>();
	ECS::ComponentManager::getInstance()->registerComponent<ECS::ModelRenderer>();
	ECS::ComponentManager::getInstance()->registerComponent<ECS::PointLight>();
	ECS::ComponentManager::getInstance()->registerComponent<ECS::ScriptComponent>();
	ECS::ComponentManager::getInstance()->registerComponent<ECS::Skeletal>();
	ECS::ComponentManager::getInstance()->registerComponent<ECS::SpotLight>();
	ECS::ComponentManager::getInstance()->registerComponent<ECS::PhysicsComponent>();
	
	RESOURCES::ServiceManager::Set<RESOURCES::ModelLoader>(modelManager);
	RESOURCES::ServiceManager::Set<RESOURCES::TextureLoader>(textureManager);
	RESOURCES::ServiceManager::Set<RESOURCES::ShaderLoader>(shaderManager);
	RESOURCES::ServiceManager::Set<RESOURCES::MaterialLoader>(materialManager);
	RESOURCES::ServiceManager::Set<INPUT_SYSTEM::InputManager>(*inputManager);
	RESOURCES::ServiceManager::Set<WINDOW_SYSTEM::Window>(*window);
	RESOURCES::ServiceManager::Set<SCENE_SYSTEM::SceneManager>(*sceneManager);
	RESOURCES::ServiceManager::Set<AUDIO::AudioManager>(*audioManager);
	RESOURCES::ServiceManager::Set<TASK::TaskSystem>(*taskManger);

	renderer = std::make_unique<RENDER::Renderer>(*driver, *this);
	renderer->setCapability(RENDER::RenderingCapability::MULTISAMPLE, true);
	RESOURCES::ServiceManager::Set<RENDER::Renderer>(*renderer);

	debugRender = std::make_unique<DEBUG::DebugRender>();

	sceneManager->getCurrentScene().init();
	
	
}

Core::~Core() = default;

#include "core.h"


#include "../config.h"
#include "../render/gameRenderer.h"
#include "../render/buffers/uniformBuffer.h"
#include "../resourceManager/materialManager.h"
#include "../resourceManager/modelManager.h"
#include "../resourceManager/ServiceManager.h"
#include "../resourceManager/shaderManager.h"
#include "../resourceManager/textureManager.h"
#include "../utils/math/Matrix4.h"
#include "../utils/math/Vector3.h"
#include "../audioManager/audioManager.h"
#include "../physics/PhysicWorld.h"

using namespace KUMA;
using namespace KUMA::CORE_SYSTEM;

Core::Core() {
	Config::Init();
	RESOURCES::ModelLoader::SetAssetPaths(Config::USER_ASSETS_PATH, Config::ENGINE_ASSETS_PATH);
	RESOURCES::TextureLoader::SetAssetPaths(Config::USER_ASSETS_PATH, Config::ENGINE_ASSETS_PATH);
	RESOURCES::ShaderLoader::SetAssetPaths(Config::USER_ASSETS_PATH, Config::ENGINE_ASSETS_PATH);
	RESOURCES::MaterialLoader::SetAssetPaths(Config::USER_ASSETS_PATH, Config::ENGINE_ASSETS_PATH);
	
	WINDOW_SYSTEM::WindowSettings windowSettings;

	window = std::make_unique<WINDOW_SYSTEM::Window>(windowSettings);
	sceneManager = std::make_unique<SCENE_SYSTEM::SceneManager>(Config::ENGINE_ASSETS_PATH);
	inputManager = std::make_unique<INPUT_SYSTEM::InputManager>(*window);
	driver = std::make_unique<GL_SYSTEM::GlManager>(GL_SYSTEM::DriverSettings{false});
	scriptInterpreter = std::make_unique<SCRIPTING::ScriptInterpreter>(Config::ROOT + Config::USER_ASSETS_PATH + "scripts\\");
	audioManager = std::make_unique<AUDIO::AudioManager>();
	physicsManger = std::make_unique<PHYSICS::PhysicWorld>(256);
	
	RESOURCES::ServiceManager::Set<RESOURCES::ModelLoader>(modelManager);
	RESOURCES::ServiceManager::Set<RESOURCES::TextureLoader>(textureManager);
	RESOURCES::ServiceManager::Set<RESOURCES::ShaderLoader>(shaderManager);
	RESOURCES::ServiceManager::Set<RESOURCES::MaterialLoader>(materialManager);
	RESOURCES::ServiceManager::Set<INPUT_SYSTEM::InputManager>(*inputManager);
	RESOURCES::ServiceManager::Set<WINDOW_SYSTEM::Window>(*window);
	RESOURCES::ServiceManager::Set<SCENE_SYSTEM::SceneManager>(*sceneManager);
	RESOURCES::ServiceManager::Set<AUDIO::AudioManager>(*audioManager);

	renderer = std::make_unique<RENDER::Renderer>(*driver, *this);
	renderer->setCapability(RENDER::RenderingCapability::MULTISAMPLE, true);
	RESOURCES::ServiceManager::Set<RENDER::Renderer>(*renderer);

	sceneManager->getCurrentScene()->init();
	
	
}

Core::~Core() = default;

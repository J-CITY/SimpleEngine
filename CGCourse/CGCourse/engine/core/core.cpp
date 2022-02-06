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

using namespace KUMA;
using namespace KUMA::CORE_SYSTEM;

Core::Core(): sceneManager(Config::ENGINE_ASSETS_PATH) {
	Config::Init();
	RESOURCES::ModelLoader::SetAssetPaths(Config::USER_ASSETS_PATH, Config::ENGINE_ASSETS_PATH);
	RESOURCES::TextureLoader::SetAssetPaths(Config::USER_ASSETS_PATH, Config::ENGINE_ASSETS_PATH);
	RESOURCES::ShaderLoader::SetAssetPaths(Config::USER_ASSETS_PATH, Config::ENGINE_ASSETS_PATH);
	RESOURCES::MaterialLoader::SetAssetPaths(Config::USER_ASSETS_PATH, Config::ENGINE_ASSETS_PATH);
	
	WINDOW_SYSTEM::WindowSettings windowSettings;

	window = std::make_unique<WINDOW_SYSTEM::Window>(windowSettings);
	inputManager = std::make_unique<INPUT_SYSTEM::InputManager>(*window);
	driver = std::make_unique<GL_SYSTEM::GlManager>(GL_SYSTEM::DriverSettings{false});
	scriptInterpreter = std::make_unique<SCRIPTING::ScriptInterpreter>(Config::ROOT + Config::USER_ASSETS_PATH + "scripts\\");
	
	RESOURCES::ServiceManager::Set<RESOURCES::ModelLoader>(modelManager);
	RESOURCES::ServiceManager::Set<RESOURCES::TextureLoader>(textureManager);
	RESOURCES::ServiceManager::Set<RESOURCES::ShaderLoader>(shaderManager);
	RESOURCES::ServiceManager::Set<RESOURCES::MaterialLoader>(materialManager);
	RESOURCES::ServiceManager::Set<INPUT_SYSTEM::InputManager>(*inputManager);
	RESOURCES::ServiceManager::Set<WINDOW_SYSTEM::Window>(*window);
	RESOURCES::ServiceManager::Set<SCENE_SYSTEM::SceneManager>(sceneManager);

	renderer = std::make_unique<RENDER::Renderer>(*driver);
	renderer->setCapability(RENDER::RenderingCapability::MULTISAMPLE, true);

	RESOURCES::ServiceManager::Set<RENDER::Renderer>(*renderer);

	sceneManager.getCurrentScene()->init();
	
	engineUBO = std::make_unique<RENDER::UniformBuffer>(
		sizeof(MATHGL::Matrix4) +
		sizeof(MATHGL::Matrix4) +
		sizeof(MATHGL::Matrix4) +
		sizeof(MATHGL::Vector3) +
		sizeof(float) +
		sizeof(MATHGL::Matrix4),
		0,
		0,
		RENDER::AccessSpecifier::STREAM_DRAW
	);

	lightSSBO = std::make_unique<RENDER::ShaderStorageBuffer>(RENDER::AccessSpecifier::STREAM_DRAW);
	lightSSBO->bind(0);
}

Core::~Core() = default;

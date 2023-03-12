#include "core.h"

#include "../config.h"
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
#include "../inputManager/inputManager.h"
#include "../scene/sceneManager.h"
#include "../debug/debugRender.h"
#include "../ecs/systems/audioSystem.h"
#include "../ecs/systems/scriptSystem.h"
#include "../ecs/systems/logicSystem.h"
#include "../ecs/systems/inputSystem.h"
#include "../resourceManager/materialManager.h"
#include "../resourceManager/modelManager.h"
#include "../resourceManager/shaderManager.h"
#include "../resourceManager/textureManager.h"
#include "../render/gameRendererInterface.h"
#include "../render/backends/interface/driverInterface.h"
#ifdef OPENGL_BACKEND
#include "../render/backends/gl/driverGl.h"
#include "../render/gameRendererGl.h"
#endif

#ifdef VULKAN_BACKEND
#include "../render/backends/vk/driverVk.h"
#include "../render/gameRendererVk.h"
#endif

#ifdef DX12_BACKEND
#include "../render/backends/dx12/driverDx12.h"
#include "../render/gameRendererDx12.h"
#endif


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


Core::Core(
#ifdef DX12_BACKEND
	HINSTANCE hInstance
#endif
)
#ifdef DX12_BACKEND
: hInstance(hInstance)
#endif
{
	RESOURCES::ModelLoader::SetAssetPaths(Config::USER_ASSETS_PATH, Config::ENGINE_ASSETS_PATH);
	RESOURCES::TextureLoader::SetAssetPaths(Config::USER_ASSETS_PATH, Config::ENGINE_ASSETS_PATH);
	RESOURCES::ShaderLoader::SetAssetPaths(Config::USER_ASSETS_PATH, Config::ENGINE_ASSETS_PATH);
	RESOURCES::MaterialLoader::SetAssetPaths(Config::USER_ASSETS_PATH, Config::ENGINE_ASSETS_PATH);

	auto res = serde::serialize<nlohmann::json>(RENDER::DriverInterface::settings);

	auto windowSettings = KUMA::UTILS::loadConfigFile<WINDOW_SYSTEM::WindowSettings>("Configs/window.json");
	if (windowSettings.isErr()) {
		LOG_ERROR(windowSettings.unwrapErr().msg);
		throw;
	}

	//auto renderSettings = KUMA::UTILS::loadConfigFile<RENDER::RenderSettings>("Configs/render.json");
	//if (renderSettings.isErr()) {
	//	LOG_ERROR(renderSettings.unwrapErr().msg);
	//	throw;
	//}

	modelManager = std::make_unique<RESOURCES::ModelLoader>();
	textureManager = std::make_unique<RESOURCES::TextureLoader>();
	shaderManager = std::make_unique<RESOURCES::ShaderLoader>();
	materialManager = std::make_unique<RESOURCES::MaterialLoader>();
	window = std::make_unique<WINDOW_SYSTEM::Window>(windowSettings.unwrap());
	sceneManager = std::make_unique<SCENE_SYSTEM::SceneManager>(Config::ENGINE_ASSETS_PATH);
	inputManager = std::make_unique<INPUT_SYSTEM::InputManager>(*window);
	RESOURCES::ServiceManager::Set<WINDOW_SYSTEM::Window>(window.get());

	//RENDER::DriverInterface::settings = renderSettings.unwrap();;
#ifdef OPENGL_BACKEND
	if (RENDER::DriverInterface::settings.backend == RENDER::RenderSettings::Backend::OPENGL) {
		driver = std::make_unique<RENDER::DriverGl>();
		driver->init();
	}
#endif
#ifdef VULKAN_BACKEND
	if (RENDER::DriverInterface::settings.backend == RENDER::RenderSettings::Backend::VULKAN) {
		driver = std::make_unique<RENDER::DriverVk>();
		driver->init();
	}
#endif

#ifdef DX12_BACKEND
	if (RENDER::DriverInterface::settings.backend == RENDER::RenderSettings::Backend::DIRECTX12) {
		driver = std::make_unique<RENDER::DriverDx12>();
		//driver->init();
	}
#endif
	if (!driver) {
		throw;
	}

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
	ECS::ComponentManager::getInstance()->registerComponent<ECS::VrCameraComponent>();
	ECS::ComponentManager::getInstance()->registerComponent<ECS::ArCameraComponent>();
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
	//GUI
	ECS::ComponentManager::getInstance()->registerComponent<ECS::RootGuiComponent>();
	ECS::ComponentManager::getInstance()->registerComponent<ECS::SpriteComponent>();
	ECS::ComponentManager::getInstance()->registerComponent<ECS::LabelComponent>();
	ECS::ComponentManager::getInstance()->registerComponent<ECS::InteractionComponent>();
	ECS::ComponentManager::getInstance()->registerComponent<ECS::ClipComponent>();
	ECS::ComponentManager::getInstance()->registerComponent<ECS::ScrollComponent>();
	ECS::ComponentManager::getInstance()->registerComponent<ECS::LayoutComponent>();

	ECS::ComponentManager::getInstance()->systemManager->registerSystem<ECS::ScriptSystem>();
	ECS::ComponentManager::getInstance()->systemManager->registerSystem<ECS::LogicSystem>();
	ECS::ComponentManager::getInstance()->systemManager->registerSystem<ECS::InputSystem>();
	ECS::ComponentManager::getInstance()->systemManager->registerSystem<ECS::AudioSystem>();
	ECS::Signature signature;
	signature.set(static_cast<unsigned int>(ECS::ComponentManager::getInstance()->getComponentType<ECS::AudioComponent>()));
	ECS::ComponentManager::getInstance()->setSystemSignature<ECS::AudioSystem>(signature);
	
	RESOURCES::ServiceManager::Set<RESOURCES::ModelLoader>(modelManager.get());
	RESOURCES::ServiceManager::Set<RESOURCES::TextureLoader>(textureManager.get());
	RESOURCES::ServiceManager::Set<RESOURCES::ShaderLoader>(shaderManager.get());
	RESOURCES::ServiceManager::Set<RESOURCES::MaterialLoader>(materialManager.get());
	RESOURCES::ServiceManager::Set<INPUT_SYSTEM::InputManager>(inputManager.get());
	RESOURCES::ServiceManager::Set<SCENE_SYSTEM::SceneManager>(sceneManager.get());
	RESOURCES::ServiceManager::Set<AUDIO::AudioManager>(audioManager.get());
	RESOURCES::ServiceManager::Set<TASK::TaskSystem>(taskManger.get());
#ifdef OPENGL_BACKEND
	if (RENDER::DriverInterface::settings.backend == RENDER::RenderSettings::Backend::OPENGL) {
		renderer = std::make_unique<RENDER::GameRendererGl>(*this);
	}
#endif
#ifdef VULKAN_BACKEND
	if (RENDER::DriverInterface::settings.backend == RENDER::RenderSettings::Backend::VULKAN) {
		renderer = std::make_unique<RENDER::GameRendererVk>(*this);
	}
#endif
#ifdef DX12_BACKEND
	if (RENDER::DriverInterface::settings.backend == RENDER::RenderSettings::Backend::DIRECTX12) {
		renderer = std::make_unique<RENDER::GameRendererDx12>(hInstance, *this);
		//driver->init();

		static_cast<RENDER::GameRendererDx12*>(renderer.get())->Initialize();
	}
#endif
	if (!renderer) {
		throw;
	}
	//renderer->setCapability(RENDER::RenderingCapability::MULTISAMPLE, true);
	RESOURCES::ServiceManager::Set<RENDER::GameRendererInterface>(static_cast<RENDER::GameRendererInterface*>(renderer.get()));
	//driver->init();

#ifdef VULKAN_BACKEND
	reinterpret_cast<RENDER::GameRendererVk*>(renderer.get())->createVkResources();
#endif
	debugRender = std::make_unique<DEBUG::DebugRender>();

	sceneManager->getCurrentScene().init();
}

Core::~Core() = default;

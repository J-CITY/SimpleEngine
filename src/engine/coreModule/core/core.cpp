#include "core.h"

#include "../config.h"
#include "../resourceManager/materialManager.h"
#include "../resourceManager/modelManager.h"
#include "../resourceManager/ServiceManager.h"
#include "../resourceManager/shaderManager.h"
#include "../resourceManager/audioSourceManager.h"
#include "../resourceManager/textureManager.h"
#include <audioModule/audioManager.h>
#include <debugModule/debugRender.h>
#include <physicsModule/PhysicWorld.h>
#include <taskModule/taskSystem.h>
#include <utilsModule/loader.h>
#include "windowModule/inputManager/inputManager.h"
#include <sceneModule/sceneManager.h>
#include "../ecs/systems/audioSystem.h"
#include "../ecs/systems/scriptSystem.h"
#include "../ecs/systems/logicSystem.h"
#include "../ecs/systems/inputSystem.h"
#include "../resourceManager/materialManager.h"
#include "../resourceManager/modelManager.h"
#include "../resourceManager/shaderManager.h"
#include "../resourceManager/textureManager.h"
#include <renderModule/gameRendererInterface.h>
#include <renderModule/backends/interface/driverInterface.h>
#ifdef OPENGL_BACKEND
#include <renderModule/backends/gl/driverGl.h>
#include <renderModule/gameRendererGl.h>
#endif

#ifdef VULKAN_BACKEND
#include <renderModule/backends/vk/driverVk.h>
#include <renderModule/gameRendererVk.h>
#endif

#ifdef DX12_BACKEND
#include <renderModule/backends/dx12/driverDx12.h>
#include <renderModule/gameRendererDx12.h>
#endif


import logger;

namespace IKIGAI
{
	namespace ECS
	{
		class InputComponent;
		class LogicComponent;
		class AmbientSphereLight;
		class AmbientLight;
	}
}

using namespace IKIGAI;
using namespace IKIGAI::CORE_SYSTEM;


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

	//auto res = serde::serialize<nlohmann::json>(RENDER::DriverInterface::settings);
	WINDOW_SYSTEM::WindowSettings windowSettings;
	//auto windowSettings = IKIGAI::UTILS::loadConfigFile<WINDOW_SYSTEM::WindowSettings>("Configs/window.json");
	//if (windowSettings.isErr()) {
	//	LOG_ERROR(windowSettings.unwrapErr().msg);
	//	throw;
	//}

	//auto renderSettings = IKIGAI::UTILS::loadConfigFile<RENDER::RenderSettings>("Configs/render.json");
	//if (renderSettings.isErr()) {
	//	LOG_ERROR(renderSettings.unwrapErr().msg);
	//	throw;
	//}

	modelManager = std::make_unique<RESOURCES::ModelLoader>();
	textureManager = std::make_unique<RESOURCES::TextureLoader>();
	shaderManager = std::make_unique<RESOURCES::ShaderLoader>();
	materialManager = std::make_unique<RESOURCES::MaterialLoader>();
	window = std::make_unique<WINDOW_SYSTEM::Window>(windowSettings);
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
	audioSourceLoader = std::make_unique<RESOURCES::AudioSourceLoader>();
	physicsManger = std::make_unique<PHYSICS::PhysicWorld>(256);
	taskManger = std::make_unique<TASK::TaskSystem>();
	taskManger->setup();

	ECS::ComponentManager::GetInstance().registerComponent<ECS::TransformComponent>();
	ECS::ComponentManager::GetInstance().registerComponent<ECS::AmbientLight>();
	ECS::ComponentManager::GetInstance().registerComponent<ECS::AmbientSphereLight>();
	ECS::ComponentManager::GetInstance().registerComponent<ECS::AudioComponent>();
	ECS::ComponentManager::GetInstance().registerComponent<ECS::AudioListenerComponent>();
	ECS::ComponentManager::GetInstance().registerComponent<ECS::CameraComponent>();
	ECS::ComponentManager::GetInstance().registerComponent<ECS::VrCameraComponent>();
	//ECS::ComponentManager::getInstance()->registerComponent<ECS::ArCameraComponent>();
	ECS::ComponentManager::GetInstance().registerComponent<ECS::DirectionalLight>();
	ECS::ComponentManager::GetInstance().registerComponent<ECS::InputComponent>();
	ECS::ComponentManager::GetInstance().registerComponent<ECS::LogicComponent>();
	ECS::ComponentManager::GetInstance().registerComponent<ECS::MaterialRenderer>();
	ECS::ComponentManager::GetInstance().registerComponent<ECS::ModelRenderer>();
	ECS::ComponentManager::GetInstance().registerComponent<ECS::PointLight>();
	ECS::ComponentManager::GetInstance().registerComponent<ECS::ScriptComponent>();
	ECS::ComponentManager::GetInstance().registerComponent<ECS::Skeletal>();
	ECS::ComponentManager::GetInstance().registerComponent<ECS::SpotLight>();
	ECS::ComponentManager::GetInstance().registerComponent<ECS::PhysicsComponent>();
	//GUI
	ECS::ComponentManager::GetInstance().registerComponent<ECS::RootGuiComponent>();
	ECS::ComponentManager::GetInstance().registerComponent<ECS::SpriteComponent>();
	ECS::ComponentManager::GetInstance().registerComponent<ECS::LabelComponent>();
	ECS::ComponentManager::GetInstance().registerComponent<ECS::InteractionComponent>();
	ECS::ComponentManager::GetInstance().registerComponent<ECS::ClipComponent>();
	ECS::ComponentManager::GetInstance().registerComponent<ECS::ScrollComponent>();
	ECS::ComponentManager::GetInstance().registerComponent<ECS::LayoutComponent>();

	ECS::ComponentManager::GetInstance().getSystemManager().registerSystem<ECS::ScriptSystem>();
	ECS::ComponentManager::GetInstance().getSystemManager().registerSystem<ECS::LogicSystem>();
	ECS::ComponentManager::GetInstance().getSystemManager().registerSystem<ECS::InputSystem>();
	ECS::ComponentManager::GetInstance().getSystemManager().registerSystem<ECS::AudioSystem>();
	ECS::Signature signature;
	signature.set(static_cast<unsigned int>(ECS::ComponentManager::GetInstance().getComponentType<ECS::AudioComponent>()));
	ECS::ComponentManager::GetInstance().setSystemSignature<ECS::AudioSystem>(signature);

	RESOURCES::ServiceManager::Set<RESOURCES::ModelLoader>(modelManager.get());
	RESOURCES::ServiceManager::Set<RESOURCES::TextureLoader>(textureManager.get());
	RESOURCES::ServiceManager::Set<RESOURCES::ShaderLoader>(shaderManager.get());
	RESOURCES::ServiceManager::Set<RESOURCES::MaterialLoader>(materialManager.get());
	RESOURCES::ServiceManager::Set<INPUT_SYSTEM::InputManager>(inputManager.get());
	RESOURCES::ServiceManager::Set<SCENE_SYSTEM::SceneManager>(sceneManager.get());
	RESOURCES::ServiceManager::Set<AUDIO::AudioManager>(audioManager.get());
	RESOURCES::ServiceManager::Set<RESOURCES::AudioSourceLoader>(audioSourceLoader.get());
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

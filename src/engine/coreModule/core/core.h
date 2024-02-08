#pragma once

#include <memory>

namespace IKIGAI
{
	namespace SCENE_SYSTEM
	{
		class SceneManager;
	}
}

namespace IKIGAI
{
	namespace RENDER
	{
		class GameRendererInterface;
		class DriverInterface;
		class GameRendererGl;
	}
}

namespace IKIGAI
{
	namespace SCRIPTING
	{
		class ScriptInterpreter;
	}
}

//namespace IKIGAI
//{
//	namespace GL_SYSTEM
//	{
//		class GlManager;
//	}
//}

namespace IKIGAI
{
	namespace RESOURCES
	{
		class AudioSourceLoader;
		class MaterialLoader;
		class ShaderLoader;
		class TextureLoader;
		class ModelLoader;
	}

	namespace DEBUG
	{
		class DebugRender;
	}
	namespace INPUT_SYSTEM
	{
		class InputManager;
	}
}

namespace IKIGAI
{
	namespace WINDOW_SYSTEM
	{
		class Window;
	}
}

namespace IKIGAI
{
	namespace TASK
	{
		class TaskSystem;
	}
}

namespace IKIGAI
{
	namespace PHYSICS
	{
		class PhysicWorld;
	}
}

namespace IKIGAI
{
	namespace AUDIO
	{
		class AudioManager;
	}
}

#ifdef DX12_BACKEND
#include <Windows.h>
#endif
namespace IKIGAI {
	namespace CORE_SYSTEM {
		class Core {
		public:
#ifdef DX12_BACKEND
			HINSTANCE hInstance;
#endif
			Core(
#ifdef DX12_BACKEND
				HINSTANCE hInstance
#endif
			);
			~Core();
		
			std::unique_ptr<WINDOW_SYSTEM::Window>        window;
			std::unique_ptr<INPUT_SYSTEM::InputManager>   inputManager;
			std::unique_ptr<RENDER::DriverInterface>         driver;
			std::unique_ptr<SCRIPTING::ScriptInterpreter> scriptInterpreter;
			std::unique_ptr<RENDER::GameRendererInterface> renderer;
			std::unique_ptr<SCENE_SYSTEM::SceneManager>   sceneManager;
			std::unique_ptr<AUDIO::AudioManager>          audioManager;
			std::unique_ptr<RESOURCES::AudioSourceLoader>     audioSourceLoader;
			std::unique_ptr<PHYSICS::PhysicWorld>         physicsManger;
			std::unique_ptr<TASK::TaskSystem>             taskManger;

			std::unique_ptr<IKIGAI::DEBUG::DebugRender> debugRender;

			std::unique_ptr<RESOURCES::ModelLoader>    modelManager;
			std::unique_ptr<RESOURCES::TextureLoader>  textureManager;
			std::unique_ptr<RESOURCES::ShaderLoader>   shaderManager;
			std::unique_ptr<RESOURCES::MaterialLoader> materialManager;
		};
	}
}

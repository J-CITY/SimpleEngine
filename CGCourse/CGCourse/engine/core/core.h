#pragma once

#include <memory>

namespace KUMA
{
	namespace SCENE_SYSTEM
	{
		class SceneManager;
	}
}

namespace KUMA
{
	namespace RENDER
	{
		class Renderer;
	}
}

namespace KUMA
{
	namespace SCRIPTING
	{
		class ScriptInterpreter;
	}
}

namespace KUMA
{
	namespace GL_SYSTEM
	{
		class GlManager;
	}
}

namespace KUMA
{
	namespace RESOURCES
	{
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

namespace KUMA
{
	namespace WINDOW_SYSTEM
	{
		class Window;
	}
}

namespace KUMA
{
	namespace TASK
	{
		class TaskSystem;
	}
}

namespace KUMA
{
	namespace PHYSICS
	{
		class PhysicWorld;
	}
}

namespace KUMA
{
	namespace AUDIO
	{
		class AudioManager;
	}
}

namespace KUMA {
	namespace CORE_SYSTEM {
		class Core {
		public:
			Core();
			~Core();
		
			std::unique_ptr<WINDOW_SYSTEM::Window>        window;
			std::unique_ptr<INPUT_SYSTEM::InputManager>   inputManager;
			std::unique_ptr<GL_SYSTEM::GlManager>         driver;
			std::unique_ptr<SCRIPTING::ScriptInterpreter> scriptInterpreter;
			std::unique_ptr<RENDER::Renderer>             renderer;
			std::unique_ptr<SCENE_SYSTEM::SceneManager>   sceneManager;
			std::unique_ptr<AUDIO::AudioManager>          audioManager;
			std::unique_ptr<PHYSICS::PhysicWorld>         physicsManger;
			std::unique_ptr<TASK::TaskSystem>             taskManger;

			std::unique_ptr<KUMA::DEBUG::DebugRender> debugRender;

			std::unique_ptr<RESOURCES::ModelLoader>    modelManager;
			std::unique_ptr<RESOURCES::TextureLoader>  textureManager;
			std::unique_ptr<RESOURCES::ShaderLoader>   shaderManager;
			std::unique_ptr<RESOURCES::MaterialLoader> materialManager;
		};
	}
}

#pragma once

#include <memory>
#include <string>

#include "../glManager/glManager.h"
#include "../inputManager/inputManager.h"
#include "../render/gameRenderer.h"
#include "../render/render.h"
#include "../render/buffers/ShaderStorageBuffer.h"
#include "../render/buffers/uniformBuffer.h"
#include "../resourceManager/materialManager.h"
#include "../resourceManager/modelManager.h"
#include "../resourceManager/shaderManager.h"
#include "../resourceManager/textureManager.h"
#include "../scene/sceneManager.h"

namespace KUMA {
	namespace CORE_SYSTEM {
		class Core {
		public:
			Core();
			~Core();

			std::unique_ptr<WINDOW_SYSTEM::Window>       window;
			std::unique_ptr<INPUT_SYSTEM::InputManager>  inputManager;
			std::unique_ptr<GL_SYSTEM::GlManager>        driver;
			std::unique_ptr<SCRIPTING::ScriptInterpreter> scriptInterpreter;
			
			std::unique_ptr<RENDER::Renderer>            renderer;
			std::unique_ptr<RENDER::UniformBuffer>       engineUBO;
			std::unique_ptr<RENDER::ShaderStorageBuffer> lightSSBO;

			SCENE_SYSTEM::SceneManager sceneManager;

			RESOURCES::ModelLoader    modelManager;
			RESOURCES::TextureLoader  textureManager;
			RESOURCES::ShaderLoader   shaderManager;
			RESOURCES::MaterialLoader materialManager;
		};
	}
}

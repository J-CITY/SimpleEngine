#pragma once
#include <unordered_map>
#include <variant>

namespace KUMA::RENDER {
	class Renderer;
}

namespace KUMA::TASK {
	class TaskSystem;
}

namespace KUMA::AUDIO {
	class AudioManager;
}

namespace KUMA::SCENE_SYSTEM {
	class SceneManager;
}

namespace KUMA::WINDOW_SYSTEM {
	class Window;
}

namespace KUMA::INPUT_SYSTEM {
	class InputManager;
}

namespace KUMA {
	namespace RESOURCES	{
		class MaterialLoader;
		class ShaderLoader;
		class TextureLoader;
		class ModelLoader;

		template <typename T>
		concept ServiceT = requires (T t) {
			std::same_as<decltype(t), RESOURCES::ModelLoader*> ||
			std::same_as<decltype(t), RESOURCES::TextureLoader*> ||
			std::same_as<decltype(t), RESOURCES::ShaderLoader*> ||
			std::same_as<decltype(t), RESOURCES::MaterialLoader*> ||
			std::same_as<decltype(t), INPUT_SYSTEM::InputManager*> ||
			std::same_as<decltype(t), WINDOW_SYSTEM::Window*> ||
			std::same_as<decltype(t), SCENE_SYSTEM::SceneManager*> ||
			std::same_as<decltype(t), AUDIO::AudioManager*> ||
			std::same_as<decltype(t), TASK::TaskSystem*> ||
			std::same_as<decltype(t), RENDER::Renderer*>;
		};

		using ServiceType = std::variant<RESOURCES::ModelLoader*, RESOURCES::TextureLoader*,
			RESOURCES::ShaderLoader*, RESOURCES::MaterialLoader*,
			INPUT_SYSTEM::InputManager*, WINDOW_SYSTEM::Window*,
			SCENE_SYSTEM::SceneManager*, AUDIO::AudioManager*,
			TASK::TaskSystem*, RENDER::Renderer*>;

		class ServiceManager {
		public:
			template<ServiceT T>
			static void Set(T& p_service) {
				SERVICES[typeid(T).hash_code()] = &p_service;
			}
			template<ServiceT T>
			static T& Get() {
				return *std::get<T*>(SERVICES[typeid(T).hash_code()]);
			}
		private:
			static std::unordered_map<size_t, ServiceType> SERVICES;
		};
	}
	
}

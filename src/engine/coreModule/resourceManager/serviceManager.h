#pragma once
#include <any>
#include <map>
#include <variant>

namespace IKIGAI::RENDER {
	class GameRendererInterface;
	class DriverInterface;
}

namespace IKIGAI::TASK {
	class TaskSystem;
}

namespace IKIGAI::AUDIO {
	class AudioManagerAL;
	class AudioManager;
}

namespace IKIGAI::SCENE_SYSTEM {
	class SceneManager;
}

namespace IKIGAI::WINDOW_SYSTEM {
	class Window;
}

namespace IKIGAI::INPUT_SYSTEM {
	class InputManager;
}

namespace IKIGAI {
	namespace RESOURCES	{
		class AudioSourceLoader;
		class MaterialLoader;
		class ShaderLoader;
		class TextureLoader;
		class ModelLoader;

		//template <typename T>
		//concept ServiceT = requires (T t) {
		//	std::same_as<decltype(t), RESOURCES::ModelLoader> ||
		//	std::same_as<decltype(t), RESOURCES::TextureLoader> ||
		//	std::same_as<decltype(t), RESOURCES::ShaderLoader> ||
		//	std::same_as<decltype(t), RESOURCES::MaterialLoader> ||
		//	std::same_as<decltype(t), INPUT_SYSTEM::InputManager> ||
		//	std::same_as<decltype(t), WINDOW_SYSTEM::Window> ||
		//	std::same_as<decltype(t), SCENE_SYSTEM::SceneManager> ||
		//	std::same_as<decltype(t), AUDIO::AudioManager> ||
		//	std::same_as<decltype(t), AUDIO::AudioManagerAL> ||
		//	std::same_as<decltype(t), TASK::TaskSystem> ||
		//	std::same_as<decltype(t), RESOURCES::AudioSourceLoader> ||
		//	std::same_as<decltype(t), RENDER::GameRendererInterface>; // ||
		//	//std::same_as<decltype(t), RENDER::DriverInterface*>;
		//};

		using ServiceType = std::any;

		class ServiceManager {
		public:
			template<class T>
			static void Set(T* p_service) {
				SERVICES[typeid(T).hash_code()] = p_service;
			}

			template<class T>
			static T& Get() {
				return *std::any_cast<T*>(SERVICES.at(typeid(T).hash_code()));
			}

			template<class T>
			static bool Check() {
				return SERVICES.contains(typeid(T).hash_code());
			}

			template<class T>
			static void Remove() {
				SERVICES.erase(typeid(T).hash_code());
			}
		private:
			static std::map<size_t, ServiceType> SERVICES;
		};
	}
}

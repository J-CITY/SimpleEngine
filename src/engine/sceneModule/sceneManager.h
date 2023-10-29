#pragma once
#include <string>

#include "scene.h"

namespace IKIGAI::SCENE_SYSTEM {
	class SceneManager {
	public:
		SceneManager(const std::string& p_sceneRootFolder = "");
		~SceneManager();

		void update();
		void loadFromFile(const std::string& sceneFilePath);
		void saveToFile(const std::string& sceneFilePath);
		void saveToFile();
		void loadEmptyScene();
		void loadDefaultScene();
		void unloadCurrentScene();
		bool hasCurrentScene() const;
		Scene& getCurrentScene();
		[[nodiscard]] std::string getCurrentSceneSourcePath() const;
		void setCurrentSceneSourcePath(const std::string& name);

	public:
		EVENT::Event<> sceneLoadEvent;
		EVENT::Event<> sceneUnloadEvent;
		EVENT::Event<const std::string&> surrentSceneSourcePathChangedEvent;

	private:
		const std::string m_sceneRootFolder;
		std::unique_ptr<Scene> m_currentScene = nullptr;

		std::string m_currentSceneSourcePath;
		std::function<void()> m_delayedLoadCall;
	};
}


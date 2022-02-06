#pragma once
#include <string>

#include "scene.h"

namespace KUMA::SCENE_SYSTEM {
	class SceneManager {
	public:
		SceneManager(const std::string& p_sceneRootFolder = "");
		~SceneManager();

		void update();

		void loadEmptyScene();
		void loadDefaultScene();
		void unloadCurrentScene();
		bool hasCurrentScene() const;
		Scene* getCurrentScene();
		std::string getCurrentSceneSourcePath() const;

	public:
		EVENT::Event<> sceneLoadEvent;
		EVENT::Event<> sceneUnloadEvent;
		EVENT::Event<const std::string&> surrentSceneSourcePathChangedEvent;

	private:
		const std::string m_sceneRootFolder;
		Scene* m_currentScene = nullptr;

		std::string m_currentSceneSourcePath = "";

		std::function<void()> m_delayedLoadCall;
	};
}


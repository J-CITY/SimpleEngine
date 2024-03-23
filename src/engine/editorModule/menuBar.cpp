#include "menuBar.h"
#ifdef USE_EDITOR
#include "imgui.h"
#include "sceneModule/sceneManager.h"

using namespace IKIGAI::EDITOR;

void MenuBar::draw() {

	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("Menu")) {
			if (ImGui::MenuItem("Save Current Scene", "CTRL+S")) {
				if (RESOURCES::ServiceManager::Get<SCENE_SYSTEM::SceneManager>().hasCurrentScene()) {
					RESOURCES::ServiceManager::Get<SCENE_SYSTEM::SceneManager>().saveToFile();
				}
			}
			if (ImGui::MenuItem("Create new scene")) {
				//popupStates["create_new_scene"] = true;
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Examples")) {
			//ImGui::MenuItem("Main menu bar", NULL, &[](){});
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
}
#endif

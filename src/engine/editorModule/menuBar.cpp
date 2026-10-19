#include "menuBar.h"
#include "prefabEditor.h"
#include "captureModule/screenshot.h"
#ifdef USE_EDITOR
#include "imgui.h"
#include "sceneModule/sceneManager.h"

using namespace IKIGAI::EDITOR;

void MenuBar::draw() {

	if (ImGui::BeginMainMenuBar()) {
        if(ImGui::BeginMenu("Capture")) {
            auto& screenshots=CAPTURE::ScreenshotService::Get();
            if(ImGui::MenuItem("Save screenshot", "F12", false, !screenshots.busy()))screenshots.request();
            const auto& status=screenshots.status();
            if(status.state==CAPTURE::ScreenshotService::State::PENDING)ImGui::TextUnformatted("Waiting for frame...");
            if(status.state==CAPTURE::ScreenshotService::State::SAVING)ImGui::TextUnformatted("Saving PNG...");
            if(status.state==CAPTURE::ScreenshotService::State::SAVED) {
                const auto utf8=status.path.u8string();
                ImGui::TextWrapped("Saved: %s",reinterpret_cast<const char*>(utf8.c_str()));
            }
            if(status.state==CAPTURE::ScreenshotService::State::FAILED)ImGui::TextWrapped("Screenshot failed: %s",status.error.c_str());
            ImGui::EndMenu();
        }
		if (ImGui::BeginMenu("Menu")) {
			if (ImGui::MenuItem("Save Current Scene", "CTRL+S")) {
				if (RESOURCES::ServiceManager::Get<SCENE_SYSTEM::SceneManager>().hasCurrentScene()) {
					PREFABS::Try([] { PREFABS::Manager().saveToFile(); });
				}
			}
			if (ImGui::MenuItem("Create new scene")) {
				//popupStates["create_new_scene"] = true;
			}
            if (ImGui::BeginMenu("Scenes")) {
                auto& manager = RESOURCES::ServiceManager::Get<SCENE_SYSTEM::SceneManager>();
#if USE_PHYSICS_3D
                if (ImGui::MenuItem("Third Person - Parkour Playground", nullptr,
                    manager.getCurrentSceneSourcePath() == "/games/third-person-traversal-demo.json"
                        || manager.getCurrentSceneSourcePath() == "games/third-person-traversal-demo.json", !manager.isEditingLocked())) {
                    PREFABS::Try([&] { manager.loadFromFileAsync("/games/third-person-traversal-demo.json"); });
                }
#endif
                if (ImGui::MenuItem("Pica Pica - Hybrid PBR", nullptr,
                    manager.getCurrentSceneSourcePath() == "/scenes/pica-pica-demo.json", !manager.isEditingLocked())) {
                    PREFABS::Try([&] { manager.loadFromFileAsync("/scenes/pica-pica-demo.json"); });
                }
                ImGui::EndMenu();
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

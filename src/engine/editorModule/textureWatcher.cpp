#include "textureWatcher.h"
#ifdef USE_EDITOR
#include "editorRender.h"
#include "IconsFontAwesome5.h"
#include "imgui.h"
#include "renderModule/backends/gl/materialGl.h"
#include "resourceModule/materialManager.h"

#include "sceneModule/sceneManager.h"
#include "misc/cpp/imgui_stdlib.h"
#include "resourceModule/textureManager.h"
#include "utilsModule/imguiHelper/imguiWidgets.h"


void IKIGAI::EDITOR::TextureWatcherWindow::draw() {
	/*
#ifdef VULKAN_BACKEND
	ImGui::Begin("Texture Watcher", nullptr);
	auto& renderer = RESOURCES::ServiceManager::Get<RENDER::GameRendererInterface>();
	auto _renderer = reinterpret_cast<RENDER::GameRendererVk*>(&renderer);

	auto winSize = RESOURCES::ServiceManager::Get<WINDOW_SYSTEM::Window>().getSize();
	ImVec2 imWinSize = ImGui::GetWindowSize();

	float w = 0.0f;
	float h = 0.0f;
	if (imWinSize.x < imWinSize.y) {
		w = imWinSize.x;
		h = (winSize.y * imWinSize.x) / winSize.x;
	} else {
		w = (imWinSize.y * winSize.x) / winSize.y;
		h = imWinSize.y;
	}
	if (h > 100.0f) {
		h -= 60.0f;
	}

	ImGui::Image((ImTextureID)_renderer->mTextures["deferredResult"]->descriptor_set, ImVec2(w, h));
	ImGui::End();
#endif


#ifdef DX12_BACKEND
	ImGui::Begin("Texture Watcher", nullptr);
	auto& renderer = RESOURCES::ServiceManager::Get<RENDER::GameRendererInterface>();
	auto _renderer = reinterpret_cast<RENDER::GameRendererDx12*>(&renderer);

	auto winSize = RESOURCES::ServiceManager::Get<WINDOW_SYSTEM::Window>().getSize();
	ImVec2 imWinSize = ImGui::GetWindowSize();

	float w = 0.0f;
	float h = 0.0f;
	if (imWinSize.x < imWinSize.y) {
		w = imWinSize.x;
		h = (winSize.y * imWinSize.x) / winSize.x;
	} else {
		w = (imWinSize.y * winSize.x) / winSize.y;
		h = imWinSize.y;
	}
	if (h > 100.0f) {
		h -= 60.0f;
	}
	auto device = RENDER::GameRendererDx12::mApp->mDriver;

	//ID3D12DescriptorHeap* dh4[] = { device->mTexturesDescHeap.Get() };
	//device->mCommandList->SetDescriptorHeaps(1, dh4);
	if (_renderer->mTextures["gAlbedoSpecTex"])
		ImGui::Image((ImTextureID)_renderer->mTextures["gAlbedoSpecTex"]->mGpuSrv.ptr, ImVec2(w, h));

	ImGui::End();
#endif

#ifdef OPENGL_BACKEND
	ImGui::Begin("Texture Watcher", nullptr);
	auto& renderer = RESOURCES::ServiceManager::Get<RENDER::GameRendererInterface>();
	auto _renderer = reinterpret_cast<RENDER::GameRendererGl*>(&renderer);


	static std::vector<std::string> items = {"Before Post Processing"};
	static bool b = false;
	if (!b) {
		items.clear();
		items.push_back("Before Post Processing");
		for (auto& e : _renderer->mTextures) {
			items.push_back(e.first);
		}
		//b = true;
	}


	static int selectedIndex = 0;
	static std::string selectedName = items[0];
	if (ImGui::BeginCombo("##textures_combo", selectedName.c_str())) {
		for (int i = 0; i < items.size(); ++i) {
			const bool isSelected = (selectedIndex == i);
			if (ImGui::Selectable(items[i].c_str(), isSelected)) {
				selectedIndex = i;
				selectedName = items[i];
			}

			// Set the initial focus when opening the combo
			// (scrolling + keyboard navigation focus)
			if (isSelected) {
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}


	auto winSize = RESOURCES::ServiceManager::Get<WINDOW_SYSTEM::Window>().getSize();
	ImVec2 imWinSize = ImGui::GetWindowSize();

	float w = 0.0f;
	float h = 0.0f;
	if (imWinSize.x < imWinSize.y) {
		w = imWinSize.x;
		h = (winSize.y * imWinSize.x) / winSize.x;
	} else {
		w = (imWinSize.y * winSize.x) / winSize.y;
		h = imWinSize.y;
	}
	if (h > 100.0f) {
		h -= 60.0f;
	}

	if (selectedIndex == 0) {
		//unsigned id = *ECS::BatchComponent::ids.begin();
		//ImGui::Image(reinterpret_cast<ImTextureID>(id), ImVec2(w, h), ImVec2(0, 1), ImVec2(1, 0));
		ImGui::Image(reinterpret_cast<ImTextureID>((uintptr_t)_renderer->mDeferredTexture->id), ImVec2(w, h), ImVec2(0, 1), ImVec2(1, 0));
	} else {
		auto tex = _renderer->mTextures[selectedName];
		if (tex->type == RENDER::TextureType::TEXTURE_3D || tex->type == RENDER::TextureType::TEXTURE_2D_ARRAY) {
			_renderer->initDebug3dTextureFB(tex);
			ImGui::Checkbox("IsRGB", &_renderer->debug3dTextureIsRGB);
			ImGui::Checkbox("IsPersp", &_renderer->debug3dTextureIsPersp);
			ImGui::SliderInt("Layer", &_renderer->debug3dTextureLayersCur, 0, _renderer->debug3dTextureLayers - 1);
			ImGui::Image(reinterpret_cast<ImTextureID>((uintptr_t)_renderer->mTextures["debug3dTexture"]->id), ImVec2(w, h), ImVec2(0, 1), ImVec2(1, 0));
		} else {
			ImGui::Image(reinterpret_cast<ImTextureID>((uintptr_t)_renderer->mTextures[selectedName]->id), ImVec2(w, h), ImVec2(0, 1), ImVec2(1, 0));
		}
	}
	ImGui::End();
#endif*/
}
#endif

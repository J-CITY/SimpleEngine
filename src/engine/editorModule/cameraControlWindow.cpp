#include "cameraControlWindow.h"

#ifdef USE_EDITOR
#ifndef OCULUS
#include "imgui.h"
#endif
#include "sceneModule/sceneManager.h"

using namespace IKIGAI::EDITOR;

void CameraControlWindow::draw() {
#ifndef OCULUS
	if (!RESOURCES::ServiceManager::Get<SCENE_SYSTEM::SceneManager>().hasCurrentScene()) {
		return;
	}
	float width = 50.0f;
	float height = 50.0f;

	static bool show_demo_window = true;
	ImGui::Begin("Camera Control", &show_demo_window);
	
	auto& curScene = RESOURCES::ServiceManager::Get<SCENE_SYSTEM::SceneManager>().getCurrentScene();
	auto cameraOpt = curScene.findMainCamera();
	if(!cameraOpt) {
		ImGui::End();
		return;
	}
	auto camera = cameraOpt.value();
	if (ImGui::Button("UP", ImVec2(width, height))) {
		auto pos = camera->obj->transform->getLocalPosition();
		pos.y += 0.3f;
		camera->obj->transform->setLocalPosition(pos);
	}
	ImGui::SameLine();
	if (ImGui::Button("FORWARD", ImVec2(width, height))) {
		auto pos = camera->obj->transform->getLocalPosition();
		pos.z += 0.3f;
		camera->obj->transform->setLocalPosition(pos);
	}
	ImGui::SameLine();
	if (ImGui::Button("DOWN", ImVec2(width, height))) {
		auto pos = camera->obj->transform->getLocalPosition();
		pos.y -= 0.3f;
		camera->obj->transform->setLocalPosition(pos);
	}

	if (ImGui::Button("LEFT", ImVec2(width, height))) {
		auto pos = camera->obj->transform->getLocalPosition();
		pos.x += 0.3f;
		camera->obj->transform->setLocalPosition(pos);
	}
	ImGui::SameLine();
	if (ImGui::Button("BACKWARD", ImVec2(width, height))) {
		auto pos = camera->obj->transform->getLocalPosition();
		pos.z -= 0.3f;
		camera->obj->transform->setLocalPosition(pos);
	}
	ImGui::SameLine();
	if (ImGui::Button("RIGHT", ImVec2(width, height))) {
		auto pos = camera->obj->transform->getLocalPosition();
		pos.x -= 0.3f;
		camera->obj->transform->setLocalPosition(pos);
	}

	if (ImGui::Button("R LEFT", ImVec2(width, height))) {
		MATH::QuaternionF addRot = MATH::QuaternionF(MATH::Vector3f(0.0, 10.0, 0.0));
		camera->obj->transform->rotateLocal(addRot);
	}
	ImGui::SameLine();
	if (ImGui::Button("R RIGHT", ImVec2(width, height))) {
		MATH::QuaternionF addRot = MATH::QuaternionF(MATH::Vector3f(0.0, -10.0, 0.0));
		camera->obj->transform->rotateLocal(addRot);
	}

	if (ImGui::Button("R UP", ImVec2(width, height))) {
		MATH::QuaternionF addRot = MATH::QuaternionF(MATH::Vector3f(-10.0, 0.0, 0.0));
		camera->obj->transform->rotateLocal(addRot);
	}
	ImGui::SameLine();
	if (ImGui::Button("R DOWN", ImVec2(width, height))) {
		MATH::QuaternionF addRot = MATH::QuaternionF(MATH::Vector3f(10.0, 0.0, 0.0));
		camera->obj->transform->rotateLocal(addRot);
	}

	ImGui::End();
#endif
}
#endif

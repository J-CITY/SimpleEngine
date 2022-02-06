#include "cameraComponent.h"

#include "../../utils/math/Vector3.h"
#include "../../render/Camera.h"

using namespace KUMA::ECS;

CameraComponent::CameraComponent(const Object& obj): Component(obj) {
	__NAME__ = "Camera";

	//renderBuffers = std::make_unique<CameraRender>();
	//renderBuffers->Init(800, 600);
	//culler = new MxEngine::FrustrumCuller();
    //this->renderTexture = std::make_shared<RESOURCES::Texture>();
    //this->renderTexture->Load(nullptr, 800, 600, 3, false, RESOURCES::TextureFormat::RGB);
    //this->renderTexture->SetWrapType(RESOURCES::TextureWrap::CLAMP_TO_EDGE);
}
void CameraComponent::ResizeRenderTexture(size_t w, size_t h) {
    //renderTexture->Load(nullptr, (int)w, (int)h, 3, false, RESOURCES::TextureFormat::RGB);
    //if (this->IsRendering())
    //    this->renderBuffers->Resize((int)w, (int)h);
}
void CameraComponent::setFov(float value) {
	camera.setFov(value);
}

void CameraComponent::setSize(float value) {
	camera.setSize(value);
}

void CameraComponent::setNear(float value) {
	camera.setNear(value);
}

void CameraComponent::setFar(float value) {
	camera.setFar(value);
}

void CameraComponent::setFrustumGeometryCulling(bool enable) {
	camera.setFrustumGeometryCulling(enable);
}

void CameraComponent::setFrustumLightCulling(bool enable) {
	camera.setFrustumLightCulling(enable);
}

void CameraComponent::setProjectionMode(RENDER::Camera::ProjectionMode p_projectionMode) {
	camera.setProjectionMode(p_projectionMode);
}

float CameraComponent::getFov() const {
	return camera.getFov();
}

float CameraComponent::getSize() const {
    return camera.getSize();
}

float CameraComponent::getNear() const {
	return camera.getNear();
}

float CameraComponent::getFar() const {
	return camera.getFar();
}

bool CameraComponent::isFrustumGeometryCulling() const {
	return camera.isFrustumGeometryCulling();
}

bool CameraComponent::isFrustumLightCulling() const {
	return camera.isFrustumLightCulling();
}

KUMA::RENDER::Camera::ProjectionMode CameraComponent::getProjectionMode() const {
    return camera.getProjectionMode();
}

KUMA::RENDER::Camera& CameraComponent::getCamera() {
	return camera;
}

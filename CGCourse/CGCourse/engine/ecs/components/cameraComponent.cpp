#include "cameraComponent.h"
#include "../../resourceManager/textureManager.h"


import glmath;
#include "../object.h"
#include "../../../game/Chunk.h"
#include "../../render/camera.h"

#include "../../resourceManager/ServiceManager.h"
#include "../../window/window.h"
using namespace KUMA::ECS;

CameraComponent::CameraComponent(Ref<ECS::Object> obj): Component(obj) {
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

VrCameraComponent::VrCameraComponent(Ref<ECS::Object> obj): Component(obj) {
	__NAME__ = "VrCamera";
	auto screenRes = RESOURCES::ServiceManager::Get<WINDOW_SYSTEM::Window>().getSize();
	leftTexture = std::make_shared<RESOURCES::Texture>(screenRes.x, screenRes.y);
	leftTexture->setFilter(RESOURCES::TextureFiltering::NEAREST, RESOURCES::TextureFiltering::NEAREST);

	rightTexture = std::make_shared<RESOURCES::Texture>(screenRes.x, screenRes.y);
	rightTexture->setFilter(RESOURCES::TextureFiltering::NEAREST, RESOURCES::TextureFiltering::NEAREST);
}

void VrCameraComponent::onUpdate(std::chrono::duration<double> dt) {
	updateEyes();
}

void VrCameraComponent::updateEyes() {
	auto& position = obj->getTransform()->getLocalPosition();

	auto LEyeTransform = left->getTransform();
	auto REyeTransform = right->getTransform();

	auto LEyeDistance = -this->EyeDistance * obj->getTransform()->getLocalRight();
	auto REyeDistance = +this->EyeDistance * obj->getTransform()->getLocalRight();

	LEyeTransform->setLocalPosition(position + LEyeDistance);
	REyeTransform->setLocalPosition(position + REyeDistance);

	auto LEyeDirection = this->FocusDistance * obj->getTransform()->getWorldPosition() - LEyeDistance;
	auto REyeDirection = this->FocusDistance * obj->getTransform()->getWorldPosition() - REyeDistance;
	
	auto screenRes = RESOURCES::ServiceManager::Get<WINDOW_SYSTEM::Window>().getSize();
	left->getComponent<CameraComponent>()->getPtr()->getCamera().cacheMatrices(screenRes.x, screenRes.y, position + LEyeDistance, obj->getTransform()->getWorldRotation());
	right->getComponent<CameraComponent>()->getPtr()->getCamera().cacheMatrices(screenRes.x, screenRes.y, position + REyeDistance, obj->getTransform()->getWorldRotation());
}



ArCameraComponent::ArCameraComponent(Ref<ECS::Object> obj) : Component(obj) {
	__NAME__ = "ArCamera";
	auto screenRes = RESOURCES::ServiceManager::Get<WINDOW_SYSTEM::Window>().getSize();
	cameraTexture = std::make_shared<RESOURCES::Texture>(screenRes.x, screenRes.y);
	cameraTexture->setFilter(RESOURCES::TextureFiltering::NEAREST, RESOURCES::TextureFiltering::NEAREST);

	cap.open(0);
	if (!cap.isOpened()) {
		std::cerr << "couldn't open capture." << std::endl;
		throw;
	}
	dictionary = getPredefinedDictionary(cv::aruco::DICT_6X6_250);
	markerDetector = cv::aruco::ArucoDetector(dictionary);

	float data[9] = { 628.158, 0., 324.099,
		0., 628.156, 260.908,
		0., 0., 1. };
	cameraMatrix = cv::Mat(3, 3, CV_8UC1, data);

	float data2[5] = { 0.0995485, -0.206384,
	   0.00754589, 0.00336531, 0 };
	distCoeffs = cv::Mat(5, 1, CV_8UC1, data2);

}

unsigned char* cvMat2TexInput(cv::Mat& img) {
	cv::cvtColor(img, img, cv::COLOR_BGR2RGBA);
	cv::flip(img, img, -1);
	return img.data;
}

cv::Matx44f createGLMatrix(const cv::Mat& tVec, const cv::Mat& rVec) {
	cv::Mat rMat;
	cv::Rodrigues(rVec, rMat);


	cv::Matx44f glM((float)rMat.at<double>(0, 0), (float)rMat.at<double>(0, 1), (float)rMat.at<double>(0, 2), (float)tVec.at<double>(0, 0),
	                (float)-rMat.at<double>(1, 0), (float)-rMat.at<double>(1, 1), (float)-rMat.at<double>(1, 2), (float)-tVec.at<double>(1, 0),
	                (float)-rMat.at<double>(2, 0), (float)-rMat.at<double>(2, 1), (float)-rMat.at<double>(2, 2), (float)-tVec.at<double>(2, 0),
	                0.0f, 0.0f, 0.0f, 1.0f);
	return glM;
}

void ArCameraComponent::onUpdate(std::chrono::duration<double> dt) {
	cap >> frame;
	//if (frame.empty())
	//	break;

	std::vector<std::vector<cv::Point2f>> markerCorners, rejectedCandidates;
	std::vector<int> markerIds;
	markerDetector.detectMarkers(frame, markerCorners, markerIds, rejectedCandidates);

	if (markerCorners.size() > 0 && markerIds.size() > 0) {

		cv::aruco::drawDetectedMarkers(frame, markerCorners, markerIds);

		int nMarkers = markerCorners.size();
		
		std::vector<cv::Vec3d> rvecs(nMarkers), tvecs(nMarkers);
		cv::aruco::estimatePoseSingleMarkers(markerCorners, 0.5, cameraMatrix, distCoeffs, rvecs, tvecs);

		markerFind = true;
		//cv::Matx44f ovm = createGLMatrix(cv::Mat(tvecs[0]), cv::Mat(tvecs[0]));

		const auto& up = MATHGL::Vector3(rvecs[0][0], rvecs[0][1], rvecs[0][2]) * MATHGL::Vector3::Up;
		const auto& forward = MATHGL::Vector3(rvecs[0][0], rvecs[0][1], rvecs[0][2]) * MATHGL::Vector3::Forward;

		view = MATHGL::Matrix4::CreateView(
			tvecs[0][0]*10, tvecs[0][1] * 10, tvecs[0][2] * 10,
			tvecs[0][0] * 10 + forward.x, tvecs[0][1] * 10 + forward.y, tvecs[0][2] * 10 + forward.z,
			up.x, up.y, up.z
		);

		//view = MATHGL::Matrix4(
		//	ovm(0, 0), ovm(0, 1), ovm(0, 2), ovm(0, 3),
		//	ovm(1, 0), ovm(1, 1), ovm(1, 2), ovm(1, 3),
		//	ovm(2, 0), ovm(2, 1), ovm(2, 2), ovm(2, 3),
		//	ovm(3, 0), ovm(3, 1), ovm(3, 2), ovm(3, 3)
		//);
		

		for (auto i : markerIds) {
			std::cout << i << " [pos: " << tvecs[0][0] << " " << tvecs[0][1] << " " << tvecs[0][2] << "] [rot "
				<< rvecs[0][0] << " " << rvecs[0][1] << " " << rvecs[0][2] << "]\n";
		}
	}
	else {
		markerFind = false;
	}

	unsigned char* image = cvMat2TexInput(frame);
	//auto screenRes = RESOURCES::ServiceManager::Get<WINDOW_SYSTEM::Window>().getSize();
	cameraTexture->bindWithoutAttach();

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, frame.cols, frame.rows, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, static_cast<GLint>(RESOURCES::TextureFiltering::LINEAR_MIPMAP_LINEAR));
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, static_cast<GLint>(RESOURCES::TextureFiltering::LINEAR));
	
	cameraTexture->unbind();

	//cv::imwrite("test.jpg", frame);
}



#include <rttr/registration>

RTTR_REGISTRATION
{
	rttr::registration::enumeration<KUMA::RENDER::Camera::ProjectionMode>("ProjectionMode")
	(
		rttr::value("ORTHOGRAPHIC",    KUMA::RENDER::Camera::ProjectionMode::ORTHOGRAPHIC),
		rttr::value("PERSPECTIVE",   KUMA::RENDER::Camera::ProjectionMode::PERSPECTIVE)
	);
	rttr::registration::class_<KUMA::ECS::CameraComponent>("CameraComponent")
	(
		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE | MetaInfo::USE_IN_EDITOR | MetaInfo::USE_IN_ANIMATION)
	)
	.property("Far", &KUMA::ECS::CameraComponent::getFar, &KUMA::ECS::CameraComponent::setFar)
	(
		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE | MetaInfo::USE_IN_EDITOR | MetaInfo::USE_IN_COMPONENT_INSPECTOR),
		rttr::metadata(EditorMetaInfo::EDIT_RANGE, Pair { 0.0f, 10000.0f }),
		rttr::metadata(EditorMetaInfo::EDIT_STEP, 0.1f),
		rttr::metadata(EditorMetaInfo::EDIT_WIDGET, EditorMetaInfo::WidgetType::DRAG_FLOAT)
	)
	.property("Fov", &KUMA::ECS::CameraComponent::getFov, &KUMA::ECS::CameraComponent::setFov)
	(
		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE | MetaInfo::USE_IN_EDITOR | MetaInfo::USE_IN_COMPONENT_INSPECTOR),
		rttr::metadata(EditorMetaInfo::EDIT_RANGE, Pair { 0.0f, 10000.0f }),
		rttr::metadata(EditorMetaInfo::EDIT_STEP, 0.1f),
		rttr::metadata(EditorMetaInfo::EDIT_WIDGET, EditorMetaInfo::WidgetType::DRAG_FLOAT)
	)
	.property("Size", &KUMA::ECS::CameraComponent::getSize, &KUMA::ECS::CameraComponent::setSize)
	(
		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE | MetaInfo::USE_IN_EDITOR | MetaInfo::USE_IN_COMPONENT_INSPECTOR),
		rttr::metadata(EditorMetaInfo::EDIT_RANGE, Pair { 0.0f, 10000.0f }),
		rttr::metadata(EditorMetaInfo::EDIT_STEP, 0.1f),
		rttr::metadata(EditorMetaInfo::EDIT_WIDGET, EditorMetaInfo::WidgetType::DRAG_FLOAT)
	)
	.property("Near", &KUMA::ECS::CameraComponent::getNear, &KUMA::ECS::CameraComponent::setNear)
	(
		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE | MetaInfo::USE_IN_EDITOR | MetaInfo::USE_IN_COMPONENT_INSPECTOR),
		rttr::metadata(EditorMetaInfo::EDIT_RANGE, Pair { 0.0f, 10000.0f }),
		rttr::metadata(EditorMetaInfo::EDIT_STEP, 0.1f),
		rttr::metadata(EditorMetaInfo::EDIT_WIDGET, EditorMetaInfo::WidgetType::DRAG_FLOAT)
	)
	.property("ProjectionMode", &KUMA::ECS::CameraComponent::getProjectionMode, &KUMA::ECS::CameraComponent::setProjectionMode)
	(
		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE | MetaInfo::USE_IN_EDITOR | MetaInfo::USE_IN_COMPONENT_INSPECTOR),
		rttr::metadata(EditorMetaInfo::EDIT_WIDGET, EditorMetaInfo::WidgetType::COMBO)
	);
}
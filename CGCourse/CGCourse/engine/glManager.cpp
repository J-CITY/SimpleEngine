

#include "glManager.h"

#include <memory>

#include "render.h"
#include "utils/debug/logger.h"
#include "utils/math/math.h"

using namespace SE;
using namespace Mathgl;

GlManager::GlManager(const Mathgl::Vec2& windowSize) {

	const GLenum error = glewInit();
	if (error != GLEW_OK) {
		std::string message = "Error Init GLEW: ";
		std::string glewError = reinterpret_cast<const char*>(glewGetErrorString(error));
		LOG_ERROR(message + glewError);
	}
	
	this->render = std::move(render);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glCullFace(GL_BACK);
	/*
	glClearColor(0.0f, 0.0f, 1.0f, 0.5f);

	glClearDepthf(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glEnable(GL_CULL_FACE);

	setDrawSize(windowSize);

	glGenBuffers(1, &lineBuffer);
	*/
}

GlManager::~GlManager(void) {
	glDeleteBuffers(1, &lineBuffer);
}

void GlManager::setDrawSize(const Vec2& size) {
	this->width = static_cast<int>(size.x);
	this->height = static_cast<int>(size.y);

	glViewport(0, 0, this->width, this->height);
}

void GlManager::bindRenderTarget(void) const {
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, this->width, this->height);
}

void GlManager::setActiveCamera(std::shared_ptr<Camera> camera) {
	activeCamera = camera;
}

Mat4 GlManager::getViewMatrix() {
	return activeCamera->getViewMatrix();
}

Mat4 GlManager::getProjectionMatrix() {
	return activeCamera->getProjectionMatrix();
}

void GlManager::draw(Object* entity) {
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	glDepthMask(GL_FALSE);
	glDepthFunc(GL_EQUAL);

	//simpleRenderer->render(*entity, activeCamera);

	glDepthFunc(GL_LESS);
	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);
}

void GlManager::renderScene(Object* obj) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	render->render(*obj, activeCamera);
}

#include "app.h"
#include "../utils/time/time.h"
#include "../physics/PhysicWorld.h"
#include "../tasks/taskSystem.h"
#include "../resourceManager/ServiceManager.h"
#include "../scene/sceneManager.h"
#include "../window/window.h"
#include "../render/gameRenderer.h"
#include "../render/buffers/uniformBuffer.h"
#include "../debug/debugRender.h"

using namespace KUMA::CORE_SYSTEM;

App::App() {
	core.sceneManager->getCurrentScene().go();
}

App::~App() = default;

void App::run() {
	while (isRunning()) {
		preUpdate();
		update(TIME::Timer::GetInstance().getDeltaTime());
		postUpdate();
		TIME::Timer::GetInstance().update();
	}
	
}

bool App::isRunning() const {
	return !core.window->isClosed();
}

void App::preUpdate() {
}

void App::update(std::chrono::duration<double> dt) {
	core.renderer->getUBO().setSubData(static_cast<float>(TIME::Timer::GetInstance().getTimeSinceStart().count()), 
		3 * sizeof(MATHGL::Matrix4) + sizeof(MATHGL::Vector3));

	auto taskUpdatePhysics = RESOURCES::ServiceManager::Get<TASK::TaskSystem>().submit("UpdatePhysics", 2, nullptr, [this]() {
		core.physicsManger->startFrame();
		auto duration = static_cast<float>(TIME::Timer::GetInstance().getDeltaTime().count());
		if (duration > 0.0f) {
			core.physicsManger->runPhysics(duration);
		}
	});

	if (core.sceneManager->hasCurrentScene()) {
		auto& currentScene = core.sceneManager->getCurrentScene();
		currentScene.fixedUpdate(dt);
		currentScene.update(dt);
		currentScene.lateUpdate(dt);
		core.renderer->renderScene();

		//core.renderer->setClearColor(1.0f, 0.0f, 0.0f);
		//core.renderer->clear(true, true, false);
	}

	RESOURCES::ServiceManager::Get<TASK::TaskSystem>().waitSync();
	
	core.sceneManager->update();
}

void App::postUpdate() {
	core.window->pollEvent();
	core.debugRender->draw(core);
	core.window->draw();
}

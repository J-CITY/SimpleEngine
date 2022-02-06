#include "app.h"
#include "../utils/time/time.h"

using namespace SE;
using namespace KUMA::CORE_SYSTEM;

App::App(): renderer(core) {
	core.sceneManager.getCurrentScene()->go();
}

App::~App() {
}

void App::run() {
	while (isRunning()) {
		preUpdate();
		update(TIME::Timer::instance()->getDeltaTime());
		postUpdate();
		core.window->update();
		TIME::Timer::instance()->update();
	}
	
}

bool App::isRunning() const {
	return !core.window->isClosed;
}

void App::preUpdate() {
}

void App::update(float dt) {
	delta += dt;

	core.engineUBO->setSubData(delta, 3 * sizeof(MATHGL::Matrix4) + sizeof(MATHGL::Vector3));

	if (auto currentScene = core.sceneManager.getCurrentScene()) {
		currentScene->fixedUpdate(dt);
		currentScene->update(dt);
		currentScene->lateUpdate(dt);
		renderer.renderScene();

		//core.renderer->setClearColor(1.0f, 0.0f, 0.0f);
		//core.renderer->clear(true, true, false);
	}

	core.sceneManager.update();
}

void App::postUpdate() {
	core.window->draw();
}

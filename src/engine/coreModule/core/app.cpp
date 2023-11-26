#include "app.h"
#include <utilsModule/time/time.h>
#include <physicsModule/PhysicWorld.h>
#include <taskModule/taskSystem.h>
#include "../resourceManager/ServiceManager.h"
#include <sceneModule/sceneManager.h>
#include <windowModule/window/window.h>
#include <renderModule/gameRendererInterface.h>
#include <debugModule/debugRender.h>
#include "../ecs/componentManager.h"
#include "../resourceManager/fileWatcher.h"

using namespace IKIGAI::CORE_SYSTEM;

App::App(): core(
#ifdef DX12_BACKEND
	hInstance
#endif
) {
	core.sceneManager->getCurrentScene().go();
}

App::~App() = default;

void App::run() {
	while (isRunning()) {
		preUpdate(TIME::Timer::GetInstance().getDeltaTime());
		update(TIME::Timer::GetInstance().getDeltaTime());
		postUpdate(TIME::Timer::GetInstance().getDeltaTime());
		TIME::Timer::GetInstance().update();
	}
	
}

bool App::isRunning() const {
	return !core.window->isClosed();
}

void App::preUpdate(std::chrono::duration<double> dt) {
#ifdef VULKAN_BACKEND
	core.debugRender->draw(core);
#endif
}

void App::update(std::chrono::duration<double> dt) {
	//core.renderer->getUBO().setSubData(static_cast<float>(TIME::Timer::GetInstance().getTimeSinceStart().count()), 
	//	3 * sizeof(MATHGL::Matrix4) + sizeof(MATHGL::Vector3));

	//auto taskUpdatePhysics = RESOURCES::ServiceManager::Get<TASK::TaskSystem>().submit("UpdatePhysics", 2, nullptr, [this]() {
	//	core.physicsManger->startFrame();
	//	auto duration = static_cast<float>(TIME::Timer::GetInstance().getDeltaTime().count());
	//	if (duration > 0.0f) {
	//		core.physicsManger->runPhysics(duration);
	//	}
	//});

	static std::chrono::duration<double> fwwait = std::chrono::milliseconds(0);
	fwwait += dt;
	if (fwwait > std::chrono::milliseconds(5000)) {
		fwwait = std::chrono::milliseconds(0);
		auto taskUpdatePhysics = RESOURCES::ServiceManager::Get<TASK::TaskSystem>().submit("UpdateFileWatcher", 2, nullptr, [this]() {
			RESOURCES::FileWatcher::getInstance()->update();
		});
	}
	RESOURCES::FileWatcher::getInstance()->applyUpdate();

	core.physicsManger->startFrame();
	auto duration = static_cast<float>(TIME::Timer::GetInstance().getDeltaTime().count());
	if (duration > 0.0f) {
		core.physicsManger->runPhysics(duration);
	}

	if (core.sceneManager->hasCurrentScene()) {
		auto& currentScene = core.sceneManager->getCurrentScene();
		currentScene.fixedUpdate(dt);
		for (auto& system : ECS::ComponentManager::GetInstance().getSystemManager().getSystems()) {
			system.second->onFixedUpdate(dt);
		}
		currentScene.update(dt);
		for (auto& system : ECS::ComponentManager::GetInstance().getSystemManager().getSystems()) {
			system.second->onUpdate(dt);
		}
		currentScene.lateUpdate(dt);
		for (auto& system : ECS::ComponentManager::GetInstance().getSystemManager().getSystems()) {
			system.second->onLateUpdate(dt);
		}
		core.renderer->renderScene();

		//core.renderer->setClearColor(1.0f, 0.0f, 0.0f);
		//core.renderer->clear(true, true, false);
	}

	//RESOURCES::ServiceManager::Get<TASK::TaskSystem>().waitSync();
	
	core.sceneManager->update();
}

void App::postUpdate(std::chrono::duration<double> dt) {
	core.window->pollEvent();
#ifdef OPENGL_BACKEND
	core.debugRender->draw(core);
#endif
	core.window->draw();
}

IKIGAI::Ref<Core> App::getCore() {
	return core;
}

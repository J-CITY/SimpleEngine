#include "app.h"
#include "uiModule/viewHost.h"
#include "captureModule/screenshot.h"
#include "coroutineModule/scheduler.h"
#include "profilerModule/profiler.h"
#include "profilerModule/profilerSpy.h"
#include <iostream>
#include <cmath>
#include <stdexcept>
#include "coreModule/ecs/componentManager.h"
#include "editorModule/editorRender.h"
#include "renderModule/gameRendererInterface.h"
#include "renderModule/render.h"
#include "resourceModule/fileWatcher.h"
#include "resourceModule/serviceManager.h"
#include "networkModule/networkRuntime.h"
#include "resourceModule/parser/assimpParser.h"
#include "sceneModule/sceneManager.h"
#include "taskModule/taskSystem.h"
#include "utilsModule/time/time.h"
#include "windowModule/window/window.h"
#include "windowModule/inputManager/inputActions.h"
#include "windowModule/inputManager/inputManager.h"
#include "ecsModule/world.h"
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif
#include "renderModule/gameRenderer.h"
#include "renderModule/backends/interface/driverInterface.h"
#include "renderModule/backends/driverFactory.h"
#ifdef USE_SDL
#include <SDL3/SDL.h>
#endif

using namespace IKIGAI::CORE;

namespace
{
	int gWebLogFrames = 8;

	void WebLog(const char* msg)
	{
#ifdef __EMSCRIPTEN__
		if (gWebLogFrames <= 0) {
			return;
		}
		std::cout << "[web] " << msg << std::endl;
		std::cout.flush();
#endif
	}

#ifdef __EMSCRIPTEN__
	void emscriptenMainLoop(IKIGAI::CORE::App* application)
	{
		try {
			application->runMainLoop();
		} catch (const std::exception& e) {
			std::cout << "[web] EXCEPTION in main loop: " << e.what() << std::endl;
			std::cout.flush();
		} catch (...) {
			std::cout << "[web] UNKNOWN EXCEPTION in main loop" << std::endl;
			std::cout.flush();
		}
	}
#endif
} // namespace


App::App(
#ifdef OCULUS
		android_app* app
#endif
		): core(
#ifdef DX12_BACKEND
	hInstance
#endif
#ifdef OCULUS
	app
#endif
) {
}

App::~App() = default;

void App::run() {
#ifdef __EMSCRIPTEN__
	std::cout << "[web] App::run, starting main loop" << std::endl;
	std::cout.flush();
	emscripten_set_main_loop_arg((em_arg_callback_func)::emscriptenMainLoop, this, 0, 1);
#else
	while (runMainLoop()) {

	}
#endif
}

bool App::runMainLoop() {
	core.network->poll();
	static int webFrame = 0;
	const int frame = webFrame++;
#ifdef __EMSCRIPTEN__
	auto step = [frame](const char* name) {
		if (frame < 8) {
			std::cout << "[web] frame " << frame << " " << name << std::endl;
			std::cout.flush();
		}
	};
#else
	auto step = [](const char*) {};
#endif

	step("enter");
	if (!isRunning()) {
		step("not running");
		return false;
	}
	auto& timer = TIME::Timer::GetInstance();
	timer.update();
	const auto frameDt = timer.getDeltaTime();
	static bool init = false;
	if (!init) {
		init = true;
	}
	step("pollEvent");
	core.window->pollEvent();
    CAPTURE::ScreenshotService::Get().status();
#if defined(USE_SDL) || defined(OCULUS)
    if(core.inputManager && core.inputManager->wasKeyPressedThisFrame(INPUT_SYSTEM::EKey::KEY_F12))
        CAPTURE::ScreenshotService::Get().request();
#endif
	
	step("preUpdate");
	{
		auto coroutineProfile = IKIGAI::PROFILER::Profiler::IsEnabled()
			? std::make_unique<IKIGAI::PROFILER::ProfilerSpy>("CoroutineScheduler::tick") : nullptr;
		core.coroutineScheduler->tick(frameDt, timer.getDeltaTimeUnscaled());
	}
	preUpdate(frameDt);
	step("update");
	update(frameDt);
	step("postUpdate");
	postUpdate(frameDt);
	step("done");
#ifdef __EMSCRIPTEN__
	if (gWebLogFrames > 0) {
		--gWebLogFrames;
	}
#endif
	return true;
}

bool App::isRunning() const {
	return !core.window->isClosed();
}

void App::preUpdate(std::chrono::duration<double> dt) {
	core.window->preUpdate();
#if USE_PHYSICS_3D && defined(USE_EDITOR)
    if(core.sceneManager->hasCurrentScene()&&!core.sceneManager->isEditingLocked())
        core.sceneManager->getCurrentScene().drawPhysics3DDebug();
#endif
#if USE_PHYSICS_2D && defined(USE_EDITOR)
    if(core.sceneManager->hasCurrentScene()&&!core.sceneManager->isEditingLocked())
        core.sceneManager->getCurrentScene().drawPhysics2DDebug();
#endif
#if USE_JSON_VIEWS && !defined(OCULUS)
    if (core.viewHost && !core.sceneManager->isEditingLocked()) core.viewHost->draw();
#endif
#ifdef USE_EDITOR
#ifndef OCULUS
	if (core.editorRender && !core.sceneManager->isEditingLocked()) {
		core.editorRender->draw();
	}
#endif
#endif
}

void App::update(std::chrono::duration<double> dt) {
    auto* frameDriver=RENDER::DriverInterface::Get();
    if(frameDriver) {
		frameDriver->latencyFrameBegin();
	}

#ifdef USE_FILE_WATCHER
	static std::chrono::duration<double> fwwait = std::chrono::milliseconds(0);
	fwwait += TIME::Timer::GetInstance().getDeltaTimeUnscaled();
	if (fwwait > std::chrono::milliseconds(RESOURCES::FileWatcher::DELAY_UPDATE)) {
		fwwait = std::chrono::milliseconds(0);
		RESOURCES::FileWatcher::getInstance()->requestUpdate();
	}
    if (!core.sceneManager->isEditingLocked() && !core.sceneManager->isPreparingLoadingScene())
        RESOURCES::FileWatcher::getInstance()->applyUpdate();
#endif

    const bool simulate = !core.sceneManager->isEditingLocked() && core.sceneManager->hasCurrentScene();
	const auto fixedDt = TIME::Timer::GetInstance().getFixedDeltaTime();
    if (simulate) m_fixedTimeAccumulator += dt;
    else m_fixedTimeAccumulator = std::chrono::duration<double>::zero();
	constexpr int kMaxFixedStepsPerFrame = 8;
	int fixedSteps = 0;
	while (m_fixedTimeAccumulator >= fixedDt && fixedSteps < kMaxFixedStepsPerFrame) {
		m_fixedTimeAccumulator -= fixedDt;
		++fixedSteps;

		if (core.sceneManager->hasCurrentScene()) {
			auto& currentScene = core.sceneManager->getCurrentScene();
			WebLog("scene fixedUpdate");
			currentScene.fixedUpdate(fixedDt);
			WebLog("scene fixedUpdate done");

			auto& world = RESOURCES::ServiceManager::Get<ECS2::World>();
			auto systemManager = world.getSystemManager();
			WebLog("ecs runFixedUpdate");
			systemManager->runFixedUpdate(fixedDt);
			WebLog("ecs runFixedUpdate done");
#if USE_PHYSICS_3D
            currentScene.stepPhysics3D(fixedDt);
#endif
#if USE_PHYSICS_2D
            currentScene.stepPhysics2D(fixedDt);
#endif
		}
	}
	if (m_fixedTimeAccumulator >= fixedDt) {
		// Drop excess whole steps under overload, but preserve the fractional remainder.
		m_fixedTimeAccumulator = std::chrono::duration<double>(
			std::fmod(m_fixedTimeAccumulator.count(), fixedDt.count()));
	}

#if defined(USE_SDL) || defined(OCULUS)
	if (simulate && RESOURCES::ServiceManager::Check<INPUT_SYSTEM::InputActions>() && RESOURCES::ServiceManager::Check<ECS2::World>()) {
		auto& actions = RESOURCES::ServiceManager::Get<INPUT_SYSTEM::InputActions>();
		actions.prepareMaps(RESOURCES::ServiceManager::Get<ECS2::World>());
		actions.resolve(dt);
	}
#endif

    bool presentationAllowed=true;
	if (simulate) {
		WebLog("scene update");
		auto& currentScene = core.sceneManager->getCurrentScene();

		auto& world = RESOURCES::ServiceManager::Get<ECS2::World>();
		auto systemManager = world.getSystemManager();
		WebLog("ecs runUpdate");
		systemManager->runUpdate(dt);
		WebLog("ecs runUpdate done");
		WebLog("scene object update");
		currentScene.update(dt);
		WebLog("scene object update done");
		WebLog("ecs runLateUpdate");
		systemManager->runLateUpdate(dt);
		WebLog("ecs runLateUpdate done");
    } else {
        core.sceneManager->updateLoadingScene(TIME::Timer::GetInstance().getDeltaTimeUnscaled());
    }
#ifndef OCULUS
		WebLog("window update");
        if(frameDriver){frameDriver->latencySimulationEnd();frameDriver->latencySubmitBegin();}
		core.window->update();
#ifdef USE_SDL
        presentationAllowed=!(SDL_GetWindowFlags(core.window->getSDLWindow())&SDL_WINDOW_MINIMIZED);
        if(!presentationAllowed){
            if(frameDriver&&frameDriver->mFrameGeneration&&frameDriver->mFrameGenerationStatus.Active){
                frameDriver->mFrameGeneration->suspend();frameDriver->mFrameGeneration->waitForPresents();
                frameDriver->mFrameGenerationStatus=frameDriver->mFrameGeneration->status();
            }
            if(auto* game=dynamic_cast<RENDER::GameRenderer*>(core.renderer.get()))game->resetTemporalHistory();
        }
#endif
		WebLog("renderScene");
		if(presentationAllowed)core.renderer->renderScene();
		WebLog("renderScene done");
#endif
#ifndef __EMSCRIPTEN__
	RESOURCES::ServiceManager::Get<TASK::TaskSystem>().waitSync();
#endif

	if (presentationAllowed&&RENDER::DriverInterface::settings.backend != RENDER::RenderSettings::Backend::OPENGL) {
#if defined(_WIN32) && defined(USE_SDL)
        RENDER::FocusPresentationForSmoke(SDL_GetPointerProperty(SDL_GetWindowProperties(core.window->getSDLWindow()),SDL_PROP_WINDOW_WIN32_HWND_POINTER,nullptr));
#endif
		RESOURCES::ServiceManager::Get<RENDER::Renderer>().submit();
	}

	//EDITOR
#ifdef USE_EDITOR
#ifndef OCULUS
#endif
#endif

#ifndef OCULUS
#ifndef VULKAN_BACKEND
	//core.window->update();
#endif
#else
	core.window->update([this](XrCompositionLayerProjectionView &layerView,
							   render_target_t &rtarget, XrPosef &stagePose,
							   uint32_t viewID) {
		static_cast<RENDER::GameRenderer*>(core.renderer.get())->renderSceneOculus(layerView, rtarget, stagePose, viewID);
	});
#endif



	WebLog("sceneManager update");
	core.sceneManager->update();
	WebLog("sceneManager update done");
}

void App::postUpdate(std::chrono::duration<double> dt) {
	WebLog("window draw");
	core.window->draw();
    core.sceneManager->notifyPresented();
	WebLog("window draw done");
#if defined(USE_SDL) || defined(OCULUS)
	if (RESOURCES::ServiceManager::Check<INPUT_SYSTEM::InputManager>()) {
		RESOURCES::ServiceManager::Get<INPUT_SYSTEM::InputManager>().endFrame();
	}
#endif
}

IKIGAI::UTILS::Ref<Core> App::getCore() {
	return core;
}

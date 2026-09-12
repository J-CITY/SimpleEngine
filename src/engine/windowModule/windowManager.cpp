#include "windowManager.h"

namespace IKIGAI::WINDOW {

	std::shared_ptr<Window> WindowManager::createMainWindow(const WindowSettings& settings) {
		auto window = std::make_shared<Window>(settings, true, nullptr);
		mMainWindowID = window->getId();
		mWindows[mMainWindowID] = window;
		return window;
	}

	std::shared_ptr<Window> WindowManager::createSecondaryWindow(const WindowSettings& settings) {
		Window* sharedWindow = nullptr;
		if (mMainWindowID != 0 && mWindows.count(mMainWindowID)) {
			sharedWindow = mWindows[mMainWindowID].get();
		}
		
		auto window = std::make_shared<Window>(settings, false, sharedWindow);
		unsigned int id = window->getId();
		mWindows[id] = window;
		return window;
	}

	std::shared_ptr<Window> WindowManager::getMainWindow() {
		if (mWindows.count(mMainWindowID)) {
			return mWindows[mMainWindowID];
		}
		return nullptr;
	}

	std::shared_ptr<Window> WindowManager::getWindow(unsigned int id) {
		if (mWindows.count(id)) {
			return mWindows[id];
		}
		return nullptr;
	}

	void WindowManager::removeWindow(unsigned int id) {
		mWindows.erase(id);
		if (id == mMainWindowID) {
			mMainWindowID = 0;
		}
	}

	void WindowManager::clear() {
		mWindows.clear();
		mMainWindowID = 0;
	}

	void WindowManager::update() {
		for (auto& [id, window] : mWindows) {
			if (!window->isClosed()) {
				window->pollEvent();
			}
		}
	}

}

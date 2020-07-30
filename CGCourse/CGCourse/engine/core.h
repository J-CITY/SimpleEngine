#pragma once;

#include "scene.h"
#include "window.h"

namespace SE {

	class Core {
	public:
		Core(std::unique_ptr<Scene> _scene) {
			scene = std::move(_scene);
		}

		void update() {
			
		}
		void go() {
			
		}
		
	private:
		std::unique_ptr<Window> m_window;
		std::chrono::high_resolution_clock::time_point time, lastTime;
		std::chrono::milliseconds deltaTime;
		
		std::unique_ptr<Scene> scene;

		bool isQuit = false;
	};
}

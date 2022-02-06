#pragma once

#include <chrono>

namespace KUMA::TIME {
	class Timer {
	public:
		void update();

		float getFPS();
		float getDeltaTime();
		float getDeltaTimeUnscaled();
		float getTimeSinceStart();
		float getTimeScale();
		void setScale(float s);

		static Timer* instance() {
			if (!singleton) {
				singleton = new Timer();
			}
			return singleton;
		}

	private:
		Timer() = default;
		
		static Timer* singleton;
		
		void init();

		std::chrono::steady_clock::time_point	start;
		std::chrono::steady_clock::time_point	last;
		std::chrono::steady_clock::time_point	current;
		std::chrono::duration<double>			eps;

		bool	isInit = false;
		float	scale = 1.0f;
		float	dt = 0.0f;
		float	allTime = 0.0f;
	};
}

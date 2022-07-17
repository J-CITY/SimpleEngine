#pragma once

#include <chrono>

namespace KUMA::TIME {
	class Timer {
	public:
		Timer(Timer const&) = delete;
		Timer& operator=(Timer const&) = delete;

		void update();
		double getFPS() const;
		std::chrono::duration<double> getDeltaTime() const;
		std::chrono::duration<double> getDeltaTimeUnscaled() const;
		std::chrono::duration<double> getTimeSinceStart() const;
		double getTimeScale() const;
		void setScale(double s);

		static Timer& GetInstance() {
			static Timer singleton;
			return singleton;
		}
	private:
		Timer();
		void init();

		std::chrono::steady_clock::time_point start;
		std::chrono::steady_clock::time_point last;
		std::chrono::steady_clock::time_point current;

		double scale = 1.0;
		std::chrono::duration<double> dt{};
		std::chrono::duration<double> allTime{};
	};
}

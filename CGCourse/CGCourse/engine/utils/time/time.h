#pragma once

#include <chrono>

namespace KUMA::TIME {
	class Timer {
	public:
		Timer(Timer const&) = delete;
		Timer& operator=(Timer const&) = delete;

		void update();
		[[nodiscard]] double getFPS() const;
		[[nodiscard]] std::chrono::duration<double> getDeltaTime() const;
		[[nodiscard]] std::chrono::duration<double> getDeltaTimeUnscaled() const;
		[[nodiscard]] std::chrono::duration<double> getTimeSinceStart() const;
		[[nodiscard]] double getTimeScale() const;
		void setScale(double s);

		static Timer& GetInstance();
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

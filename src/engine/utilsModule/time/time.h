#pragma once

#include <chrono>

namespace IKIGAI::TIME {
	class Timer {
	public:
		Timer(Timer const&) = delete;
		Timer& operator=(Timer const&) = delete;

		// Sample once at frame start. The first sample has a zero delta.
		void update();
		[[nodiscard]] double getFPS() const;
		// Capped at 0.1 seconds before applying the time scale.
		[[nodiscard]] std::chrono::duration<double> getDeltaTime() const;
		// Actual elapsed wall time, without a cap or time scale.
		[[nodiscard]] std::chrono::duration<double> getDeltaTimeUnscaled() const;
		// Constant simulation step in game seconds, independent of time scale.
		[[nodiscard]] std::chrono::duration<double> getFixedDeltaTime() const;
		// Wall time since construction, independent of time scale.
		[[nodiscard]] std::chrono::duration<double> getTimeSinceStart() const;
		[[nodiscard]] double getTimeScale() const;
		// Invalid values are ignored; zero scale pauses game time.
		void setScale(double s);
		// Only finite, positive simulation steps are accepted.
		void setFixedDeltaTime(std::chrono::duration<double> step);

		static Timer& GetInstance();
	private:
		Timer();

		std::chrono::steady_clock::time_point start;
		std::chrono::steady_clock::time_point lastFrameTime;
		bool firstFrame = true;
		double scale = 1.0;
		std::chrono::duration<double> dt{};
		std::chrono::duration<double> fixedDt{1.0 / 50.0};
		static constexpr std::chrono::duration<double> maxFrameDt{0.1};
	};
}

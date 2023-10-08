#include "time.h"

using namespace IKIGAI::TIME;

Timer::TimerGenerator tick() {
	std::chrono::steady_clock::time_point current = std::chrono::steady_clock::now();
	std::chrono::steady_clock::time_point prevTime = current;
	while (true) {
		prevTime = current;
		current = std::chrono::steady_clock::now();
		co_yield current - prevTime;
	}
}

Timer::Timer(): timerGenerator(tick()) {
	init();
}

double Timer::getFPS() const {
	return 1.0 / (dt.count());
}

std::chrono::duration<double> Timer::getDeltaTime() const {
	return dt * scale;
}

std::chrono::duration<double> Timer::getDeltaTimeUnscaled() const {
	return dt;
}

std::chrono::duration<double> Timer::getTimeSinceStart() const {
	return std::chrono::steady_clock::now() - start;
}

double Timer::getTimeScale() const {
	return scale;
}

void Timer::setScale(double s) {
	scale = s;
}

Timer& Timer::GetInstance() {
	static Timer singleton;
	return singleton;
}

void Timer::init() {
	start = std::chrono::steady_clock::now();
}

void Timer::update() {
	dt = timerGenerator.h_.promise().value_;
	timerGenerator.h_();
}

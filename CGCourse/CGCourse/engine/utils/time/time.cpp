#include "time.h"

using namespace KUMA::TIME;

Timer::Timer() {
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
	return allTime;
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
	current = start;
	last = start;
}

void Timer::update() {
	last = current;
	current = std::chrono::steady_clock::now();
	dt = current - last;
	allTime += dt * scale;
}

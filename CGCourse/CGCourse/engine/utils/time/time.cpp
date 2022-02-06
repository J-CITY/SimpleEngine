
#include "time.h"

using namespace KUMA::TIME;

Timer* Timer::singleton = nullptr;

float Timer::getFPS() {
	return 1.0f / (dt);
}

float Timer::getDeltaTime() {
	return dt * scale;
}

float Timer::getDeltaTimeUnscaled() {
	return dt;
}

float Timer::getTimeSinceStart() {
	return allTime;
}

float Timer::getTimeScale() {
	return scale;
}

void Timer::setScale(float s) {
	scale = s;
}

void Timer::init() {
	dt = 0.0f;
	start = std::chrono::steady_clock::now();
	current = start;
	last = start;
	isInit = true;
}

void Timer::update() {
	last = current;
	current = std::chrono::steady_clock::now();
	eps = current - last;

	if (isInit) {
		dt = eps.count() > 0.1 ? 0.1f : static_cast<float>(eps.count());
		allTime += dt * scale;
	}
	else {
		init();
	}
}

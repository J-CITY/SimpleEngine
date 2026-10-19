#include "time.h"
#include <algorithm>
#include <cmath>

using namespace IKIGAI::TIME;

Timer::Timer()
	: start(std::chrono::steady_clock::now())
	, lastFrameTime(start) { }

double Timer::getFPS() const {
	return dt.count() > 0.0 ? 1.0 / dt.count() : 0.0;
}

std::chrono::duration<double> Timer::getDeltaTime() const {
	return std::min(dt, maxFrameDt) * scale;
}

std::chrono::duration<double> Timer::getDeltaTimeUnscaled() const {
	return dt;
}

std::chrono::duration<double> Timer::getFixedDeltaTime() const {
	return fixedDt;
}

void Timer::setFixedDeltaTime(std::chrono::duration<double> step) {
	if (std::isfinite(step.count()) && step.count() > 0.0) {
		fixedDt = step;
	}
}

std::chrono::duration<double> Timer::getTimeSinceStart() const {
	return std::chrono::steady_clock::now() - start;
}

double Timer::getTimeScale() const {
	return scale;
}

void Timer::setScale(double s) {
	if (std::isfinite(s) && s >= 0.0) {
		scale = s;
	}
}

Timer& Timer::GetInstance() {
	static Timer singleton;
	return singleton;
}

void Timer::update() {
	const auto now = std::chrono::steady_clock::now();
	dt = firstFrame ? std::chrono::duration<double>::zero() : now - lastFrameTime;
	lastFrameTime = now;
	firstFrame = false;
}

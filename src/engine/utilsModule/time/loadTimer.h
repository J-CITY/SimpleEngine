#pragma once
#include "coreModule/config.h"
#include <chrono>
#include <iostream>
#include <string>

namespace IKIGAI::UTILS {
// Opt-in timings include suspension time for asynchronous loading operations.
class LoadTimer {
	using Clock = std::chrono::steady_clock;
	const char* mStage;
	std::string mPath;
	Clock::time_point mStart;
public:
	explicit LoadTimer(const char* stage, std::string path = {})
		: mStage(stage), mPath(std::move(path)), mStart(Config::PROFILE_LOADING ? Clock::now() : Clock::time_point{}) {}
	~LoadTimer() {
		if (Config::PROFILE_LOADING)
			std::cout << "LOAD_TIMING stage=" << mStage << " ms="
				<< std::chrono::duration<double, std::milli>(Clock::now() - mStart).count()
				<< " path=" << mPath << std::endl;
	}
};
}

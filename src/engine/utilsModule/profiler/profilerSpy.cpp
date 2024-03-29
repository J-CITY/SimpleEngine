#include "profilerSpy.h"

#include "profiler.h"

using namespace IKIGAI;
using namespace IKIGAI::PROFILER;

ProfilerSpy::ProfilerSpy(const std::string & p_name) :
	mName(p_name),
	mStart(std::chrono::steady_clock::now()),
	mThreadId(std::hash<std::thread::id>{}(std::this_thread::get_id())) {
	Profiler::IncreaseLevel();
}

ProfilerSpy::~ProfilerSpy() {
	mEnd = std::chrono::steady_clock::now();
	Profiler::Save(*this);
	Profiler::DecreaseLevel();
}

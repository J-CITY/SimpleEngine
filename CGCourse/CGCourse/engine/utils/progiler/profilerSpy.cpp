#include "profilerSpy.h"

#include "profiler.h"

using namespace KUMA;
using namespace KUMA::PROFILER;

ProfilerSpy::ProfilerSpy(const std::string & p_name) :
	mName(p_name),
	mStart(std::chrono::steady_clock::now()),
	mThreadId(std::hash<std::thread::id>{}(std::this_thread::get_id())) {

}

ProfilerSpy::~ProfilerSpy() {
	mEnd = std::chrono::steady_clock::now();
	Profiler::Save(*this);
}

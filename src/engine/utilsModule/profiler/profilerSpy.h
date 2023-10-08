#pragma once

#include <string>
#include <chrono>

#define PROFILER_EVENT()\
		std::unique_ptr<IKIGAI::PROFILER::ProfilerSpy> __profiler_spy__ = \
		IKIGAI::PROFILER::Profiler::IsEnabled() ? std::make_unique<IKIGAI::PROFILER::ProfilerSpy>(__FUNCTION__) : nullptr

namespace IKIGAI::PROFILER {
	struct ProfilerSpy {
		ProfilerSpy(const std::string& p_name);
		~ProfilerSpy();

		const std::string mName;
		const unsigned mThreadId = 0;
		const std::chrono::steady_clock::time_point	mStart;
		std::chrono::steady_clock::time_point mEnd;
	};
}
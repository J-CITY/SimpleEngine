#pragma once

#include <string>
#include <chrono>

#define PROFILER_EVENT()\
		std::unique_ptr<KUMA::PROFILER::ProfilerSpy> __profiler_spy__ = \
		KUMA::PROFILER::Profiler::IsEnabled() ? std::make_unique<KUMA::PROFILER::ProfilerSpy>(__FUNCTION__) : nullptr

namespace KUMA::PROFILER {
	struct ProfilerSpy {
		ProfilerSpy(const std::string& p_name);
		~ProfilerSpy();

		const std::string mName;
		const unsigned mThreadId = 0;
		const std::chrono::steady_clock::time_point	mStart;
		std::chrono::steady_clock::time_point mEnd;
	};
}
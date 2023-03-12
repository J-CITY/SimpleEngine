#pragma once

#include <string>
#include <vector>

namespace KUMA::PROFILER {
	struct ProfilerReport {
		struct Action {
			std::string mName;
			double mDuration = 0.0;
			double mPercentage = 0.0;
			uint64_t mCalls = 0;
		};

		struct Node {
			std::string mName;
			double mDuration = 0.0;
			int level = 0;
		};

		double				mTime		= 0.0;
		uint32_t			mFrames	= 0u;
		std::vector<Action> mActions;

		std::unordered_map<std::thread::id, std::vector<ProfilerReport::Node>> mCallsTree;
	};
}

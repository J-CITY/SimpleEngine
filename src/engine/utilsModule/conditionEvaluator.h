#pragma once

#include "environment.h"
#include "renderModule/backends/interface/resourceStruct.h"

namespace IKIGAI::UTILS {
	class ConditionEvaluator {
	public:
		static bool Evaluate(const RENDER::ConditionVar& cond, const Environment& env);
	};
}

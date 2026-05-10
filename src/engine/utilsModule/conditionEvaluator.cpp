#include "conditionEvaluator.h"

namespace IKIGAI::UTILS {
	bool ConditionEvaluator::Evaluate(const RENDER::ConditionVar& cond, const Environment& env) {
		switch (cond.type) {
		case RENDER::ConditionType::COMPARE: {
			if (!env.hasVariable(cond.varLeft)) return false;

			VarValue leftValRaw = env.getValue(cond.varLeft);
			float leftVal = 0.0f;
			if (std::holds_alternative<float>(leftValRaw)) leftVal = std::get<float>(leftValRaw);
			else if (std::holds_alternative<int>(leftValRaw)) leftVal = static_cast<float>(std::get<int>(leftValRaw));
			else if (std::holds_alternative<bool>(leftValRaw)) leftVal = std::get<bool>(leftValRaw) ? 1.0f : 0.0f;

			float rightVal = cond.constRight;
			if (cond.isRightVar) {
				if (!env.hasVariable(cond.varRight)) return false;
				VarValue rightValRaw = env.getValue(cond.varRight);
				if (std::holds_alternative<float>(rightValRaw)) rightVal = std::get<float>(rightValRaw);
				else if (std::holds_alternative<int>(rightValRaw)) rightVal = static_cast<float>(std::get<int>(rightValRaw));
				else if (std::holds_alternative<bool>(rightValRaw)) rightVal = std::get<bool>(rightValRaw) ? 1.0f : 0.0f;
			}

			switch (cond.op) {
			case RENDER::ConditionOp::EQUAL: return leftVal == rightVal;
			case RENDER::ConditionOp::NOT_EQUAL: return leftVal != rightVal;
			case RENDER::ConditionOp::LESS: return leftVal < rightVal;
			case RENDER::ConditionOp::GREATER: return leftVal > rightVal;
			case RENDER::ConditionOp::LESS_EQUAL: return leftVal <= rightVal;
			case RENDER::ConditionOp::GREATER_EQUAL: return leftVal >= rightVal;
			default: return false;
			}
		}
		case RENDER::ConditionType::LOGICAL_AND: {
			for (const auto& child : cond.children) {
				if (!Evaluate(child, env)) return false;
			}
			return true;
		}
		case RENDER::ConditionType::LOGICAL_OR: {
			if (cond.children.empty()) return false;
			for (const auto& child : cond.children) {
				if (Evaluate(child, env)) return true;
			}
			return false;
		}
		case RENDER::ConditionType::LOGICAL_NOT: {
			if (cond.children.empty()) return true;
			return !Evaluate(cond.children[0], env);
		}
		case RENDER::ConditionType::SCRIPT: {
			// TODO: Call script function here
			return false;
		}
		default:
			return false;
		}
	}
}

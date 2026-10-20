#pragma once
#include "expression.h"
#include <serdepp/serde.hpp>

namespace IKIGAI::DATA {
struct LiteralDescriptor {
	ValueType Type = ValueType::Float;
	float Float = 0.f;
	int Int = 0;
	bool Bool = false;
	MATH::Vector3f Vector3 {};

	Value value() const;
	template <class C> static auto serde(C& context, LiteralDescriptor& descriptor) {
		using namespace serde::attribute;
		serde::serde_struct(context, descriptor)
			.field(&LiteralDescriptor::Type, "Type", default_ { ValueType::Float })
			.field(&LiteralDescriptor::Float, "Float", default_ { 0.f })
			.field(&LiteralDescriptor::Int, "Int", default_ { 0 })
			.field(&LiteralDescriptor::Bool, "Bool", default_ { false })
			.field(&LiteralDescriptor::Vector3, "Vector3", default_ { MATH::Vector3f {} });
	}
};

struct ExpressionDescriptor {
	Op Operation = Op::Constant;
	std::string Variable;
	LiteralDescriptor Constant;
	std::vector<ExpressionDescriptor> Arguments;

	template <class C> static auto serde(C& context, ExpressionDescriptor& descriptor) {
		using namespace serde::attribute;
		serde::serde_struct(context, descriptor)
			.field(&ExpressionDescriptor::Operation, "Operation")
			.field(&ExpressionDescriptor::Variable, "Variable", default_ { std::string {} })
			.field(&ExpressionDescriptor::Constant, "Constant", default_ { LiteralDescriptor {} })
			.field(&ExpressionDescriptor::Arguments, "Arguments", default_ { std::vector<ExpressionDescriptor> {} });
	}
};
}

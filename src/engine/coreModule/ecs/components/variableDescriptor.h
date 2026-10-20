#pragma once
#include "coreModule/ecs/componentReference.h"
#include "utilsModule/expressions/expressionDescriptor.h"
#include <optional>

namespace IKIGAI::ECS {
enum class VariableType { Float, Int, Bool, Vector3, Event };

struct ScriptVariableDescriptor {
	std::string Path;
	std::string Function;
	std::vector<std::string> Arguments;
	template <class C> static auto serde(C& context, ScriptVariableDescriptor& descriptor) {
		serde::serde_struct(context, descriptor)
			.field(&ScriptVariableDescriptor::Path, "Path")
			.field(&ScriptVariableDescriptor::Function, "Function")
			.field(&ScriptVariableDescriptor::Arguments, "Arguments", serde::attribute::default_ { std::vector<std::string> {} });
	}
};

struct VariableDescriptor {
	VariableType Type = VariableType::Float;
	std::optional<DATA::LiteralDescriptor> Default;
	std::optional<PropertyReference> Property;
	std::optional<DATA::ExpressionDescriptor> Expression;
	std::optional<ScriptVariableDescriptor> Script;

	DATA::Value initialValue() const;
	template <class C> static auto serde(C& context, VariableDescriptor& descriptor) {
		serde::serde_struct(context, descriptor)
			.field(&VariableDescriptor::Type, "Type")
			.field(&VariableDescriptor::Default, "Default")
			.field(&VariableDescriptor::Property, "Property")
			.field(&VariableDescriptor::Expression, "Expression")
			.field(&VariableDescriptor::Script, "Script");
	}
};
}

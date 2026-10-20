#include "variableDescriptor.h"
#include <stdexcept>

namespace IKIGAI::ECS {
DATA::Value VariableDescriptor::initialValue() const {
	const auto type = Type == VariableType::Event ? DATA::ValueType::Bool : static_cast<DATA::ValueType>(Type);
	if (Default) {
		if (Default->Type != type) {
			throw std::runtime_error("Variable default does not match its declared type");
		}
		return Default->value();
	}
	switch (type) {
	case DATA::ValueType::Float:
		return 0.f;
	case DATA::ValueType::Int:
		return 0;
	case DATA::ValueType::Bool:
		return false;
	case DATA::ValueType::Vector3:
		return MATH::Vector3f {};
	}
	throw std::runtime_error("Invalid variable type");
}
}

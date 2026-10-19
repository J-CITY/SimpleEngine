#pragma once
#include <nlohmann/json.hpp>
#include <stdexcept>
namespace IKIGAI::PHYSICS2D {
inline void validateFeatures(const nlohmann::json& data) {
#if !USE_PHYSICS_2D
	if (data.is_object())
		for (const char* key : { "Physics2DComponentType", "SoftBody2DComponentType", "Fluid2DComponentType", "Joint2DComponentType",
			 "ForceField2DComponentType", "Cloth2DComponentType" })
			if (data.contains(key))
				throw std::runtime_error("Scene requires 2D physics, but USE_PHYSICS_2D=OFF");
	if (data.is_structured())
		for (const auto& item : data)
			validateFeatures(item);
#else
	(void)data;
#endif
}
}

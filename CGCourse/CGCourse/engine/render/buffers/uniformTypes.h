#pragma once
#include <cstdint>
#include <variant>
#include <memory>
#include "../../utils/math/vector2.h"
#include "../../utils/math/vector3.h"
#include "../../utils/math/vector4.h"
#include "../../resourceManager/resource/texture.h"


namespace KUMA {
	namespace RENDER {
		enum class AccessSpecifier {
			STREAM_DRAW = 0x88E0,
			STREAM_READ = 0x88E1,
			STREAM_COPY = 0x88E2,
			DYNAMIC_DRAW = 0x88E8,
			DYNAMIC_READ = 0x88E9,
			DYNAMIC_COPY = 0x88EA,
			STATIC_DRAW = 0x88E4,
			STATIC_READ = 0x88E5,
			STATIC_COPY = 0x88E6
		};
		enum class UniformType : unsigned int {
			UNIFORM_BOOL = 0x8B56,
			UNIFORM_INT = 0x1404,
			UNIFORM_FLOAT = 0x1406,
			UNIFORM_FLOAT_VEC2 = 0x8B50,
			UNIFORM_FLOAT_VEC3 = 0x8B51,
			UNIFORM_FLOAT_VEC4 = 0x8B52,
			UNIFORM_FLOAT_MAT4 = 0x8B5C,
			UNIFORM_DOUBLE_MAT4 = 0x8F48,
			UNIFORM_SAMPLER_2D = 0x8B5E,
			UNIFORM_SAMPLER_CUBE = 0x8B60
		};

		using ShaderUniform = std::variant<float, int, bool, MATHGL::Vector2f, MATHGL::Vector3, MATHGL::Vector4, std::shared_ptr<RESOURCES::Texture>>;

		struct UniformInfo {
			RENDER::UniformType type;
			std::string name;
			unsigned location;
			ShaderUniform defaultValue;
		};
	}
}

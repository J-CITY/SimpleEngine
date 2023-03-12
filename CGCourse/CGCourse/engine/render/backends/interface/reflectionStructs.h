#pragma once
#include <string>
#include <vector>

namespace KUMA::RENDER {
	enum SHADER_TYPE {
		NONE,
		VERTEX = 0x00000001,
		FRAGMENT = 0x00000010,
		GEOMETRY = 0x00000008,
		TESSELLATION_CONTROL = 0x00000002,
		TESSELLATION_EVALUATION = 0x00000004,
		COMPUTE = 0x00000020
	};
	enum class UNIFORM_TYPE {
		NONE, MAT4, MAT3, VEC4, VEC3, VEC2, INT, FLOAT, BOOL, SAMPLER_2D, SAMPLER_3D, SAMPLER_CUBE, SAMPLER_2D_ARRAY
	};
	struct UniformInform {
		enum class TYPE {
			NONE,
			UNIFORM,
			UNIFORM_SAMPLER2D,
			UNIFORM_BUFFER,
			STORAGE_BUFFER,
			SPV_REFLECT_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR,
			SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE
		};
		struct Member {
			UNIFORM_TYPE type = UNIFORM_TYPE::NONE;
			std::string name;
			int offset = 0;
			int size = 0;
			int arraySize = 0;
		};
		TYPE type = TYPE::NONE;
		size_t binding = 0;
		size_t set = 0;
		int size = 0;
		unsigned shaderType = 0;
		std::string name;
		std::vector<Member> members;
	};
}

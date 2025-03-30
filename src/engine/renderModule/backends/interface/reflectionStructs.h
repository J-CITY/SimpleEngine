#pragma once
#include <string>
#include <vector>

#include "renderEnums.h"

namespace IKIGAI::RENDER {
	enum class ShaderType: int {
		NONE = 0,
		VERTEX = 1,
		FRAGMENT = 2,
		GEOMETRY = 4,
		TESSELLATION_CONTROL = 8,
		TESSELLATION_EVALUATION = 16,
		COMPUTE = 32
	};

	enum class UNIFORM_TYPE {
		NONE, MAT4, MAT3, VEC4, VEC3, VEC2, INT, FLOAT, BOOL, SAMPLER_2D,
#ifndef USING_GLES
		SAMPLER_3D,
#endif
		SAMPLER_CUBE,
#ifndef USING_GLES
		SAMPLER_2D_ARRAY,
		IMAGE_3D
#endif
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

	struct ShaderReflection {
		struct InputParam {
			std::string mName;
			size_t mIndex = 0;
			size_t mLocation = 0;
			PixelFormat mFormat = PixelFormat::RGBA_FLOAT;
			size_t mOffset = 0;
			size_t mSize = 0;
		};
		enum class UniformType {
			NONE, MAT4, MAT3, VEC4, VEC3, VEC2, INT, FLOAT, BOOL,
			SAMPLER_2D,
			SAMPLER_CUBE,
			SAMPLER_3D,
			SAMPLER_2D_ARRAY,
			UNIFORM_BUFFER,
			STORAGE_BUFFER,
			IMAGE_2D,
			IMAGE_3D,
			IMAGE_2D_ARRAY,
			IMAGE_CUBE,
			//TODO: use bits and make TEXTURE mask
		};
		struct UniformMember {
			std::string mName;
			UniformType mType = UniformType::NONE;
			int mOffset = 0;
			int mSize = 0;
			int mArraySize = 0;
		};
		struct Uniform {
			std::string mName;
			UniformType mType = UniformType::NONE;
			size_t mRootId = 0;
			size_t mBind = 0;
			size_t mSet = 0;
			size_t mShaderMask = 0;
			size_t mSize = 0;
			std::vector<UniformMember> mMembers;

		};
		struct OutputParam {
			PixelFormat mFormat = PixelFormat::RGBA_FLOAT;
		};
		std::vector<InputParam> mInputParams;
		std::vector<OutputParam> mOutputParam;
		std::vector<Uniform> mUniforms;
		std::map<std::string, size_t> mNameToUniforms;
	};
}

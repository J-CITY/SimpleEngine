#pragma once
#ifdef DX12_BACKEND
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
namespace IKIGAI::RENDER {
	class SimpleHLSLReflection {
	public:
		enum class ShaderType {
			VERTEX,
			FRAGMENT
		};

		enum class VarType {
			UNKNOWN = -1,
			FLOAT,
			INT,
			UINT,
			VEC2,
			VEC3,
			VEC4,
			MAT3,
			MAT4,
			STRUCT
		};

		enum class UniformType {
			UNKNOWN = -1,
			UNIFORM,
			TEXTURE_2D,
			TEXTURE_3D,
			TEXTURE_CUBE,
			UNIFORM_BUFFER,
			STRUCTURED_BUFFER,
			SAMPLER_STATE
		};

		SimpleHLSLReflection(const std::string& path, const std::string& entryPoint, ShaderType type);

		struct StructMemberDescriptor {
			std::string name;
			VarType type;
			int arrSize = 0;
			std::string semantic;
		};
		struct StructDescriptor {
			std::string name;
			std::vector<StructMemberDescriptor> members;
		};

		struct UniformMemberDescriptor {
			std::string name;
			VarType type;
			int arrSize = 0;
		};
		struct UniformDescriptor {
			std::string name;
			UniformType type;
			std::string subtypeName; // for StructuredBuffer DX12
			VarType subtypeType; // for StructuredBuffer DX12
			size_t bind = 0;
			size_t space = 0;
			std::vector<UniformMemberDescriptor> members;
		};


		std::optional<StructDescriptor> vertexDescriptor = std::nullopt;
		std::optional<StructDescriptor> colorDescriptor = std::nullopt;
		std::unordered_map<std::string, StructDescriptor> structsDescriptors;
		std::unordered_map<std::string, UniformDescriptor> uniformsDescriptors;

	private:
		std::string m_folder;

		std::unordered_map<std::string, VarType> strToVerType = {
			{ "float", VarType::FLOAT },
			{ "int", VarType::INT },
			{ "uint", VarType::UINT },
			{ "float2", VarType::VEC2 },
			{ "float3", VarType::VEC3 },
			{ "float4", VarType::VEC4 },
			{ "float3x3", VarType::MAT3 },
			{ "float4x4", VarType::MAT4 }
		};
		VarType getVarType(const std::string& str) {
			if (strToVerType.count(str)) {
				return strToVerType[str];
			}
			return VarType::STRUCT;
		}

		std::unordered_map<std::string, UniformType> strToUniformType = {
			{ "Texture2D", UniformType::TEXTURE_2D },
			{ "TextureCube", UniformType::TEXTURE_CUBE },
			{ "cbuffer", UniformType::UNIFORM_BUFFER },
			{ "StructuredBuffer", UniformType::STRUCTURED_BUFFER },
			{ "SamplerState", UniformType::SAMPLER_STATE }
		};
		UniformType getUniformType(const std::string& str) {
			if (strToUniformType.count(str)) {
				return strToUniformType[str];
			}
			throw;
			//return UniformType::STRUCT;
		}




		std::unordered_map<std::string, std::string> defines;


		void readFileLines(const std::string& path, std::list<std::string>& curShaderLines) const;
		void extractInformationFromShaderFile(const std::string& path, const std::string& entryPoint, ShaderType type);
	};
}
#endif
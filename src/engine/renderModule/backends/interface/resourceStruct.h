#pragma once
#include <optional>
#include <string>
#include <vector>
#include <serdepp/serializer.hpp>
#include <serdepp/attribute/make_optional.hpp>

#include "renderEnums.h"
#include "textureInterface.h"
#include "mathModule/math.h"
#include "resourceModule/parser/parseFlags.h"
//#include "coreModule/resourceManager/parser/assimpParser.h"


namespace IKIGAI::RENDER {
	enum class ResourceType {
		SHADER,
		TEXTURE,
		MODEL,
		AUDIO,
		MATERIAL
	};

	enum class TextureResMode
	{
		IMAGE,
		COLOR,
		DEPTH,
		EMPTY
	};
	
	struct TextureResource {
		ResourceType type = ResourceType::TEXTURE;
		std::string path;
		bool needFileWatch = false;

		TextureType texType = TextureType::TEXTURE_2D;

		std::vector<std::string> pathTexture;
		std::vector<uint8_t> colorData;

		int width = 0;
		int height = 0;
		int depth = 0;
		PixelDataFormat pixelType = PixelDataFormat::RGBA;


		bool isFloat = false;
		bool useMipmap = true;

		template<class Context>
		constexpr static auto serde(Context& context, TextureResource& value) {
			using Self = TextureResource;
			using namespace serde::attribute;
			serde::serde_struct(context, value)
				.field(&Self::needFileWatch, "NeedFileWatch", default_{true})
				.field(&Self::texType, "TexType", default_{TextureType::TEXTURE_2D})
				.field(&Self::pathTexture, "PathTexture", default_{std::vector<std::string>()})
				.field(&Self::colorData, "ColorData", default_{std::vector<uint8_t>()})
				.field(&Self::width, "Width", default_{0})
				.field(&Self::height, "Height", default_{0})
				.field(&Self::depth, "Depth", default_{0})
				.field(&Self::pixelType, "PixelType", default_{PixelDataFormat::RGBA})
				.field(&Self::isFloat, "IsFloat", default_{false})
				.field(&Self::useMipmap, "UseMipmap", default_{true});
		}
	};

	struct ShaderResource {
		ResourceType type = ResourceType::SHADER;
		std::string path;
		bool needFileWatch = false;

		bool useBinary = false;

		std::string vertex;
		std::string fragment;
		std::string geometry;
		std::string tessControl;
		std::string tessEval;
		std::string compute;

		template<class Context>
		constexpr static auto serde(Context& context, ShaderResource& value) {
			using Self = ShaderResource;
			using namespace serde::attribute;
			serde::serde_struct(context, value)
				.field(&Self::needFileWatch, "NeedFileWatch", default_{true})
				.field(&Self::useBinary, "UseBinary", default_{false})
				.field(&Self::vertex, "Vertex", default_{""})
				.field(&Self::fragment, "Fragment", default_{""})
				.field(&Self::geometry, "Geometry", default_{""})
				.field(&Self::tessControl, "TessControl", default_{""})
				.field(&Self::tessEval, "TessEval", default_{""})
				.field(&Self::compute, "Compute", default_{""});
		}
	};

	struct ModelResource {
		ResourceType type = ResourceType::MODEL;
		std::string path;
		bool needFileWatch = false;

		std::string pathModel;
		std::vector<RESOURCES::ModelParserFlags> flags;

		template<class Context>
		constexpr static auto serde(Context& context, ModelResource& value) {
			using Self = ModelResource;
			using namespace serde::attribute;
			serde::serde_struct(context, value)
				.field(&Self::needFileWatch, "NeedFileWatch", default_{true})
				.field(&Self::pathModel, "PathModel")
				.field(&Self::flags, "Flags", default_{std::vector<RESOURCES::ModelParserFlags>()});
		}
	};

	struct MaterialResource {
		ResourceType Type = ResourceType::MATERIAL;
		std::string path;
		bool NeedFileWatch = false;

		std::string ShaderPath;
		bool Blendable = false;
		bool BackfaceCulling = true;
		bool FrontfaceCulling = false;
		bool DepthTest = true;
		bool DepthWriting = true;
		bool ColorWriting = true;
		int GpuInstances = 1;
		bool IsDeferred = false;
		ComparaisonAlgorithm DepthFunc = ComparaisonAlgorithm::LESS;

		using UniformType = std::variant<float, int, bool, std::string, MATH::Vector4f, MATH::Vector3f, MATH::Vector2f>;

		std::map<std::string, UniformType> Uniforms;

		template<class Context>
		constexpr static auto serde(Context& context, MaterialResource& value) {
			using Self = MaterialResource;
			using namespace serde::attribute;
			serde::serde_struct(context, value)
				.field(&Self::NeedFileWatch, "NeedFileWatch", default_{true})
				.field(&Self::ShaderPath, "ShaderPath", default_{""})
				.field(&Self::Blendable, "Blendable", default_{false})
				.field(&Self::BackfaceCulling, "BackfaceCulling", default_{true})
				.field(&Self::FrontfaceCulling, "FrontfaceCulling", default_{false})
				.field(&Self::DepthTest, "DepthTest", default_{true})
				.field(&Self::DepthWriting, "DepthWriting", default_{true})
				.field(&Self::ColorWriting, "ColorWriting", default_{true})
				.field(&Self::GpuInstances, "GpuInstances", default_{1})
				.field(&Self::IsDeferred, "IsDeferred", default_{false})
				.field(&Self::DepthFunc, "DepthFunc", default_{ComparaisonAlgorithm::LESS})
				.field(&Self::Uniforms, "Uniforms");
		}
	};

}

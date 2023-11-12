#pragma once
#include <optional>
#include <string>

#include "renderEnums.h"
#include "textureInterface.h"
#include "coreModule/resourceManager/parser/assimpParser.h"

import glmath;

namespace IKIGAI::RENDER {
	enum class ResourceType {
		SHADER,
		TEXTURE,
		MODEL,
		AUDIO
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

		std::string pathTexture;
		bool useColor = false;
		std::vector<uint8_t> colorData;

		int width = 0;
		int height = 0;
		int depth = 0;
		PixelDataFormat pixelType;


		bool isFloat = false;
		bool useMipmap = true;
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
	};

	struct ModelResource {
		ResourceType type = ResourceType::MODEL;
		std::string path;
		bool needFileWatch = false;

		std::string pathModel;
		std::vector<RESOURCES::ModelParserFlags> flags;
	};

}

#include "textureGl.h"

#include <array>
#include <filesystem>
#include <vector>

#include "utilsModule/jsonLoader.h"
#include "utilsModule/pathGetter.h"
#include "utilsModule/stdLoader.h"

//#include "coreModule/ecs/components/transform.h"


#ifdef OPENGL_BACKEND
#include <iostream>
#include <coreModule/graphicsWrapper.hpp>
//#include <coreModule/resourceManager/textureManager.h>
#include <renderModule/backends/interface/resourceStruct.h>

using namespace IKIGAI;
using namespace IKIGAI::RENDER;

TextureGl::TextureGl(const TextureResource &descriptor, const std::vector<void *> &data) {
	create(descriptor, data);
}

void TextureGl::create(const TextureResource &descriptor, const std::vector<void *> &data) {
	int chanels = 4;
	auto getFormat = [](int nrComponents) {
	GLenum format = GL_RGBA;
		if (nrComponents == 1)
//#ifndef USING_GLES
			format = GL_RED;
//#else
//			format = GL_ALPHA;
//#endif
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;
		return format;
	};
	auto getInternalFormat = [](int nrComponents, bool isFloat) {
		GLenum format = GL_RGBA;
		if (nrComponents == 1)
//#ifndef USING_GLES
			format = isFloat ? GL_R16F : GL_RED;
//#else
//			format = GL_ALPHA;
//#endif
		else if (nrComponents == 3)
//#ifndef USING_GLES
			format = isFloat ? GL_RGB16F : GL_RGB;
//#else
//			format = GL_RGB;
//#endif
		else if (nrComponents == 4)
//#ifndef USING_GLES
			format = isFloat ? GL_RGBA16F : GL_RGBA;
//#else
//			format = GL_RGBA;
//#endif
		return format;
	};

	auto getFormat2 = [&](PixelFormat dataFormat) {
		GLenum format = GL_RGBA;
		if (dataFormat == PixelFormat::R_INT || dataFormat == PixelFormat::R_FLOAT) {
			format = GL_RED;
			chanels = 1;
		}
		// #else
		//	 if (dataFormat == PixelDataFormat::ALPHA) {
		//	   format = GL_ALPHA;
		//	   chanels = 1;
		//	 }
		// #endif
		else if (dataFormat == PixelFormat::RGB_INT || dataFormat == PixelFormat::RGB_FLOAT) {
			format = GL_RGB;
			chanels = 3;
		}
		else if (dataFormat == PixelFormat::RGBA_INT || dataFormat == PixelFormat::RGBA_FLOAT) {
			format = GL_RGBA;
			chanels = 4;
		}
		else if (dataFormat == PixelFormat::DEPTH_24_UNORM_STENCIL_8_UINT) {
			format = GL_DEPTH_COMPONENT;
			chanels = 1;
		}
		else if (dataFormat == PixelFormat::DEPTH32_FLOAT) {
			// GL_DEPTH_COMPONENT32F : GL_DEPTH_COMPONENT;
			format = GL_DEPTH_COMPONENT;
			chanels = 1;
		}
		else if (dataFormat == PixelFormat::DEPTH32_FLOAT_S8X24_UINT) {
			format = GL_DEPTH_COMPONENT;
			chanels = 1;
		}
		else if (dataFormat == PixelFormat::DEPTH_32_FLOAT_STENCIL_8_UINT) {
			format = GL_DEPTH_COMPONENT;
			chanels = 1;
		}
		return format;
	};
	auto getInternalFormat2 = [&](PixelFormat dataFormat, bool isFloat) {
		GLenum format = GL_RGBA;
		// #ifndef USING_GLES
		if (dataFormat == PixelFormat::R_INT ||
			dataFormat == PixelFormat::R_FLOAT) {
			format = isFloat ? GL_R16F : GL_RED;
			chanels = 1;
		}
		//#else
		//if (dataFormat == PixelDataFormat::ALPHA) {
		//	format = GL_ALPHA;
		//	chanels = 1;
		//}
		//#endif
		else if (dataFormat == PixelFormat::RGB_INT || dataFormat == PixelFormat::RGB_FLOAT) {
			format = isFloat ? GL_RGB16F : GL_RGB;
			chanels = 3;
		}
		else if (dataFormat == PixelFormat::RGBA_INT || dataFormat == PixelFormat::RGBA_FLOAT) {
			format = isFloat ? GL_RGBA16F : GL_RGBA;
			chanels = 4;
		}
		else if (dataFormat == PixelFormat::DEPTH_24_UNORM_STENCIL_8_UINT) {
			format = GL_DEPTH24_STENCIL8;
			chanels = 1;
		}
		else if (dataFormat == PixelFormat::DEPTH32_FLOAT) {
			// GL_DEPTH_COMPONENT32F : GL_DEPTH_COMPONENT;
			format = GL_DEPTH_COMPONENT32F;
			chanels = 1;
		}
		else if (dataFormat == PixelFormat::DEPTH32_FLOAT_S8X24_UINT) {
			format = GL_FLOAT_32_UNSIGNED_INT_24_8_REV;
			chanels = 1;
		}
		else if (dataFormat == PixelFormat::DEPTH_32_FLOAT_STENCIL_8_UINT) {
			format = GL_DEPTH32F_STENCIL8;
			chanels = 1;
		}
		return format;
	};

	auto getType = [](TextureType type) {
		switch (type) {
		case TextureType::TEXTURE_2D:
			return GL_TEXTURE_2D;
	#ifndef USING_GLES
		case TextureType::TEXTURE_3D:
			return GL_TEXTURE_3D;
	#endif
		case TextureType::TEXTURE_CUBE:
			return GL_TEXTURE_CUBE_MAP;
	//#ifndef USING_GLES
		case TextureType::TEXTURE_2D_ARRAY:
			return GL_TEXTURE_2D_ARRAY;
	//#endif
		default:
			return GL_TEXTURE_2D;
		}
		return GL_TEXTURE_2D;
	};
	auto createTexture = [](TextureType type, int internalFormat, int format, int width, int height, int depth, bool isFloat, const std::vector<void *> &datas) {
		switch (type) {
		case TextureType::TEXTURE_2D: {
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, (isFloat ? GL_FLOAT : GL_UNSIGNED_BYTE), datas[0]);
		}
		return;
		case TextureType::TEXTURE_3D: {
	#ifndef USING_GLES
			glTexImage3D(GL_TEXTURE_3D, 0, internalFormat, width, height, depth, 0, format, (isFloat ? GL_FLOAT : GL_UNSIGNED_BYTE), datas[0]);
	#endif
		}
		return;
		case TextureType::TEXTURE_CUBE: {
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			for (unsigned int i = 0; i < 6; ++i) {
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internalFormat, width, height, 0, format, (isFloat ? GL_FLOAT : GL_UNSIGNED_BYTE), datas[i]);
		  }
		}
		return;
		case TextureType::TEXTURE_2D_ARRAY: {
	//#ifndef USING_GLES
			glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, internalFormat, width, height, depth, 0, format, (isFloat ? GL_FLOAT : GL_UNSIGNED_BYTE), datas[0]);
	//#endif
		}
		return;
		}
	};

	unsigned texId = 0;
	glGenTextures(1, &texId);
	glBindTexture(getType(descriptor.texType), texId);
	if (!descriptor.pathTexture.empty()) {
		UTILS::STBiSetFlipVerticallyOnLoad(true);
		int width = descriptor.width, height = descriptor.height, nrComponents = descriptor.channels;
		createTexture(descriptor.texType, getInternalFormat(nrComponents, descriptor.isFloat),
		getFormat(nrComponents), width, height, descriptor.depth,
			descriptor.isFloat, data);
		mWidth = width;
		mHeight = height;
		mDepth = descriptor.depth;
		mChannels = chanels;
	} else {
		if (descriptor.isFloat) {
			UTILS::STBiSetFlipVerticallyOnLoad(true);
		}
		const uint8_t *data = nullptr;
		if (!descriptor.colorData.empty()) {
			data = descriptor.colorData.data();
		}
		std::vector<void *> datas;
		datas.push_back((void *)data);
		createTexture(descriptor.texType, getInternalFormat2(descriptor.pixelType, descriptor.isFloat),
			getFormat2(descriptor.pixelType), descriptor.width, descriptor.height, descriptor.depth, descriptor.isFloat, datas);
		mWidth = descriptor.width;
		mHeight = descriptor.height;
		mDepth = descriptor.depth;
	}

	if (descriptor.useMipmap) {
		glGenerateMipmap(getType(descriptor.texType));
	}
	glBindTexture(getType(descriptor.texType), 0);

	id = texId;
	mPath = descriptor.path;
	mType = descriptor.texType;
}

TextureGl::~TextureGl() {
	glDeleteTextures(1, &id);
}


//std::shared_ptr<TextureGl> TextureGl::Create(const std::string& path, bool generateMipmap) {
//	auto tex = std::make_shared<TextureGl>();
//
//	unsigned int textureID;
//	glGenTextures(1, &textureID);
//
//	int width=0, height=0, nrComponents=0;
//
//	IKIGAI::UTILS::STBiSetFlipVerticallyOnLoad(true);
//	//stbi_set_flip_vertically_on_load(true);
//	unsigned char* data = IKIGAI::UTILS::STBiLoad(UTILS::GetRealPath(path).c_str(), &width, &height, &nrComponents, 0);
//	if (data)
//	{
//		GLenum format = GL_RGBA;
//		if (nrComponents == 1)
//#ifndef USING_GLES
//			format = GL_RED;
//#else
//			format = GL_ALPHA;
//#endif
//		else if (nrComponents == 3)
//			format = GL_RGB;
//		else if (nrComponents == 4)
//			format = GL_RGBA;
//
//		glBindTexture(GL_TEXTURE_2D, textureID);
//		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
//		if (generateMipmap) {
//			glGenerateMipmap(GL_TEXTURE_2D);
//		}
//		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//
//		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR); // Use GL_NEAREST_MIPMAP_LINEAR if you want to use mipmaps
//		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//		
//		IKIGAI::UTILS::STBiImageFree(data);
//	}
//	else {
//		std::cout << "Texture failed to load at path: " << path << std::endl;
//		IKIGAI::UTILS::STBiImageFree(data);
//	}
//
//	tex->id = textureID;
//	tex->mPath = path;
//	tex->mWidth = width;
//	tex->mHeight = height;
//	tex->mChannels = nrComponents;
//	return tex;
//}

std::shared_ptr<TextureGl>
TextureGl::Create(const TextureResource &descriptor) {
	auto &_descriptor = const_cast<TextureResource &>(descriptor);
	// Load data
	std::vector<void *> textureData;
	if (!_descriptor.pathTexture.empty()) {
		IKIGAI::UTILS::STBiSetFlipVerticallyOnLoad(true);
		for (const auto &path : _descriptor.pathTexture) {
			int width = 0, height = 0, channels = 0;
			if (_descriptor.isFloat) {
				auto *data = IKIGAI::UTILS::STBiLoadf(UTILS::GetRealPath(path).c_str(), &width, &height, &channels, 0);
				textureData.push_back(data);
			}
			else {
				auto *data = IKIGAI::UTILS::STBiLoad(UTILS::GetRealPath(path).c_str(), &width, &height, &channels, 0);
				//if (channels == 3) { // because dx12 dose not support RGB8
				//	UTILS::STBiImageFree((unsigned char *)data);
				//	data = IKIGAI::UTILS::STBiLoad(UTILS::GetRealPath(path).c_str(), &width, &height, &channels, 4);
				//}
				textureData.push_back(data);
			}
			_descriptor.width = width;
			_descriptor.height = height;
			_descriptor.channels = channels;
		}
	}
	else if (!_descriptor.colorData.empty()) {
		textureData.push_back(_descriptor.colorData.data());
	}

	// Create texture
	auto tex = std::make_shared<TextureGl>(descriptor, textureData);

	// Free texture data
	if (!_descriptor.pathTexture.empty()) {
		for (auto data : textureData) {
			if (_descriptor.isFloat) {
				UTILS::STBiImageFree((float *)data);
			}
			else {
				UTILS::STBiImageFree((unsigned char *)data);
			}
		}
	}
	return tex;
}

std::shared_ptr<TextureGl> TextureGl::Create(const std::string& path, bool genMipmap) {
	//int width = 0, height = 0, nrComponents = 0;
	//IKIGAI::UTILS::STBiSetFlipVerticallyOnLoad(true);
	//unsigned char* data = IKIGAI::UTILS::STBiLoad(UTILS::GetRealPath(path).c_str(), &width, &height, &nrComponents, 4);
	TextureResource res;
	res.useMipmap = genMipmap;
	res.pathTexture.push_back(path);
	//auto tex = std::make_shared<TextureDx12>(res, std::vector<void*>{data});
	return Create(res);
}

void TextureGl::bind(int _slot) {
  slot = _slot;
  glActiveTexture(GL_TEXTURE0 + slot);
  if (mType == TextureType::TEXTURE_2D) {
	glBindTexture(GL_TEXTURE_2D, id);
  } else if (mType == TextureType::TEXTURE_CUBE) {
	glBindTexture(GL_TEXTURE_CUBE_MAP, id);
  } else if (mType == TextureType::TEXTURE_2D_ARRAY) {
//#ifndef USING_GLES
		glBindTexture(GL_TEXTURE_2D_ARRAY, id);
//#endif
	}
	else if (mType == TextureType::TEXTURE_3D) {
#ifndef USING_GLES
		glBindTexture(GL_TEXTURE_3D, id);
#endif
	}
}

void TextureGl::unbind() {
	glActiveTexture(GL_TEXTURE0);
}

void TextureGl::generateMips() {
	if (mType == TextureType::TEXTURE_2D) {
		glBindTexture(GL_TEXTURE_2D, id);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else if (mType == TextureType::TEXTURE_CUBE) {
		glBindTexture(GL_TEXTURE_CUBE_MAP, id);
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	}
	else if (mType == TextureType::TEXTURE_3D) {
#ifndef USING_GLES
		glBindTexture(GL_TEXTURE_3D, id);
		glGenerateMipmap(GL_TEXTURE_3D);
#endif
	}
}

void TextureGl::bindImage(uint32_t unit, uint32_t mip_level, uint32_t layer, unsigned access, unsigned format) {
	glActiveTexture(GL_TEXTURE0 + unit);
#ifndef USING_GLES
	glBindTexture(GL_TEXTURE_3D, id);

	if (mType == TextureType::TEXTURE_3D)
		glBindImageTexture(unit, id, mip_level, GL_TRUE, layer, access, format);
	else
		glBindImageTexture(unit, id, mip_level, GL_FALSE, 0, access, format);
#endif
}

void* TextureGl::getImguiId() {
	return reinterpret_cast<void*>(id);
}


//---------------------------------------

AtlasRect TextureAtlas::getPiece(const std::string& name) const {
	if (mAtlas.mRects.contains(name)) {
		return mAtlas.mRects.at(name);
	}
	return AtlasRect();
}

AtlasRect TextureAtlas::getPieceUV(const std::string& name) const {
	if (mAtlas.mRects.contains(name)) {
		auto res = mAtlas.mRects.at(name);
		res.mX /= mWidth;
		res.mY /= mHeight;
		res.mW /= mWidth;
		res.mH /= mHeight;
		return res;
	}
	return AtlasRect();
}

//TODO: refactor it
std::shared_ptr<TextureAtlas> TextureAtlas::CreateAtlas(const std::string& path, bool generateMipmap) {
	auto tex = std::make_shared<TextureAtlas>();

	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width = 0, height = 0, nrComponents = 0;

	//IKIGAI::UTILS::stbiSetFlipVerticallyOnLoad(true);
	//stbi_set_flip_vertically_on_load(true);
	unsigned char* data = UTILS::STBiLoad(path.c_str(), &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
//#ifndef USING_GLES
			format = GL_RED;
//#else
//			format = GL_ALPHA;
//#endif
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		if (generateMipmap) {
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		IKIGAI::UTILS::STBiImageFree(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		IKIGAI::UTILS::STBiImageFree(data);
	}

	tex->id = textureID;
	tex->mPath = path;
	tex->mWidth = width;
	tex->mHeight = height;
	tex->mChannels = nrComponents;

	std::filesystem::path configPath{ path };
	configPath.replace_extension(".atlas");

	//const std::string jsonData = UTILS::readFileIntoString(UTILS::getRealPath(configPath.string()));
	//auto json = nlohmann::json::parse(jsonData, nullptr, true, true);
	//
	//AtlasData adata;
	//adata.mPath = json["Path"];
	//for (auto& e : json["Files"]) {
	//	std::string key = e["key"];
	//	AtlasRect val(e["value"]["x"], e["value"]["y"], e["value"]["w"], e["value"]["h"]);
	//	adata.mRects.insert({ key, val });
	//}
	//tex->mAtlas = adata;

	//TODO: why is not work
	auto res = UTILS::FromJson<AtlasData>(configPath.string());
	if (res.isOk()) {
		tex->mAtlas = res.unwrap();
	}
	else {
		throw;
	}
	return tex;
}


#endif

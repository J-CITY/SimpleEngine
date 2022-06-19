#include "textureManager.h"
#include "resource/texture.h"
#define STB_IMAGE_IMPLEMENTATION
#include "ServiceManager.h"
#include "../../3rd/stb/stb_image.h"

using namespace KUMA;
using namespace KUMA::RESOURCES;

void RESOURCES::stbiSetFlipVerticallyOnLoad(bool b) {
	stbi_set_flip_vertically_on_load(b);
}

float* RESOURCES::stbiLoadf(char const* filename, int* x, int* y, int* channels_in_file, int desired_channels) {
	return stbi_loadf(filename, x, y, channels_in_file, desired_channels);
}

void RESOURCES::stbiImageFree(float* data) {
	stbi_image_free(data);
}

ResourcePtr<Texture> TextureLoader::Create(const std::string& filepath, TextureFiltering firstFilter, TextureFiltering secondFilter, bool generateMipmap) {
	//GLuint textureID;
	int textureWidth;
	int textureHeight;
	int bitsPerPixel;
	//glGenTextures(1, &textureID);

	stbi_set_flip_vertically_on_load(true);
	unsigned char* dataBuffer = stbi_load(filepath.c_str(), &textureWidth, &textureHeight, &bitsPerPixel, 4);

	if (dataBuffer) {
		auto res = CreateFromMemory(dataBuffer, textureWidth, textureHeight, firstFilter, secondFilter, generateMipmap);
		stbi_image_free(dataBuffer);
		return res;
	}
	else {
		stbi_image_free(dataBuffer);
		glBindTexture(GL_TEXTURE_2D, 0);
		return nullptr;
	}
}

ResourcePtr<Texture> TextureLoader::CreateFloat(const std::string& filepath, TextureFiltering firstFilter, TextureFiltering secondFilter, bool generateMipmap) {
	//GLuint textureID;
	int textureWidth;
	int textureHeight;
	int bitsPerPixel;
	//glGenTextures(1, &textureID);

	stbi_set_flip_vertically_on_load(true);
	float* dataBuffer = stbi_loadf(filepath.c_str(), &textureWidth, &textureHeight, &bitsPerPixel, 0);

	if (dataBuffer) {
		auto res = CreateFromMemoryFloat(dataBuffer, textureWidth, textureHeight, firstFilter, secondFilter, generateMipmap);
		stbi_image_free(dataBuffer);
		return res;
	}
	else {
		stbi_image_free(dataBuffer);
		glBindTexture(GL_TEXTURE_2D, 0);
		return nullptr;
	}
}

ResourcePtr<Texture> TextureLoader::CreateColor(uint8_t r, uint8_t g, uint8_t b, TextureFiltering firstFilter, TextureFiltering secondFilter, bool generateMipmap) {
	uint8_t buffer [] = {r, g, b, 255};
	return CreateFromMemory(&buffer[0], 1, 1, firstFilter, secondFilter, generateMipmap);
}

ResourcePtr<Texture> TextureLoader::CreateColor(uint32_t data, TextureFiltering firstFilter, TextureFiltering secondFilter, bool generateMipmap) {
	unsigned char bytes[4];
	bytes[0] = (data >> 24) & 0xFF;
	bytes[1] = (data >> 16) & 0xFF;
	bytes[2] = (data >> 8) & 0xFF;
	bytes[3] = data & 0xFF;
	return CreateFromMemory(&bytes[0], 1, 1, firstFilter, secondFilter, generateMipmap);
}

std::shared_ptr<CubeMap> TextureLoader::CreateColorCM(uint8_t r, uint8_t g, uint8_t b) {
	auto cubemap = std::make_shared<CubeMap>();
	uint8_t buffer [] = {r, g, b};
	std::array<uint8_t*, 6> sides = {buffer, buffer, buffer, buffer, buffer, buffer};
	cubemap->Load(sides, 1, 1);
	return cubemap;
}

ResourcePtr<Texture> TextureLoader::CreateFromMemory(uint8_t* data, uint32_t width, uint32_t height, TextureFiltering firstFilter, TextureFiltering secondFilter, bool generateMipmap) {
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

	if (generateMipmap) {
		glGenerateMipmap(GL_TEXTURE_2D);
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, static_cast<GLint>(firstFilter));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, static_cast<GLint>(secondFilter));

	glBindTexture(GL_TEXTURE_2D, 0);

	return ResourcePtr<Texture>(new Texture("", textureID, width, height, 32, firstFilter, secondFilter, generateMipmap), [](Texture* m) {
		ServiceManager::Get<TextureLoader>().unloadResource<TextureLoader>(m->path);
	});
}

ResourcePtr<Texture> TextureLoader::CreateFromMemoryFloat(float* data, uint32_t width, uint32_t height, TextureFiltering firstFilter, TextureFiltering secondFilter, bool generateMipmap) {
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data);

	if (generateMipmap) {
		glGenerateMipmap(GL_TEXTURE_2D);
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, static_cast<GLint>(firstFilter));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, static_cast<GLint>(secondFilter));

	glBindTexture(GL_TEXTURE_2D, 0);

	return std::make_shared<Texture>("", textureID, width, height, 32, firstFilter, secondFilter, generateMipmap);
}

std::unique_ptr<CubeMap> TextureLoader::CreateSkybox(const std::string& path) {
	auto sb = std::make_unique<CubeMap>(path);
	glGenTextures(1, &sb->id);
	glBindTexture(GL_TEXTURE_CUBE_MAP, sb->id);

	for (unsigned int i = 0; i < sb->types.size(); i++) {
		GLuint textureID;
		int textureWidth;
		int textureHeight;
		int bitsPerPixel;
		stbi_set_flip_vertically_on_load(true);
		unsigned char* dataBuffer = stbi_load((path + std::to_string(i) + ".bmp").c_str(), &textureWidth, &textureHeight, &bitsPerPixel, 4);
		glTexImage2D(sb->types[i], 0, GL_RGB, textureWidth, textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, dataBuffer);
		stbi_image_free(dataBuffer);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	return sb;
}

std::unique_ptr<CubeMap> TextureLoader::LoadCubeMap(const int shadow_width, const int shadow_height) {
	auto sb = std::make_unique<CubeMap>("");
	glGenTextures(1, &sb->id);
	glBindTexture(GL_TEXTURE_CUBE_MAP, sb->id);

	for (unsigned int i = 0; i < sb->types.size(); ++i) {
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
			0, GL_DEPTH_COMPONENT, shadow_width, shadow_height, 0,
			GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	return sb;
}


std::shared_ptr<CubeMap> TextureLoader::CreateCubeMap(const int width, const int height,
	TextureFiltering firstFilter, TextureFiltering secondFilter) {
	auto cm = std::make_shared<CubeMap>();
	//glGenTextures(1, &sb->id);
	//glBindTexture(GL_TEXTURE_CUBE_MAP, sb->id);
	cm->bindWithoutAttach();
	for (unsigned int i = 0; i < cm->types.size(); ++i) {
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
			0, GL_RGB16F, width, height, 0,
			GL_RGB, GL_FLOAT, NULL);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, static_cast<GLint>(firstFilter));
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, static_cast<GLint>(secondFilter));
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	return cm;
}

ResourcePtr<Texture> TextureLoader::createResource(const std::string& path) {
	return CreateFromFile(path);
}

ResourcePtr<Texture> TextureLoader::CreateFromFile(const std::string& path) {
	std::string realPath = getRealPath(path);

	auto [min, mag, mipmap] = std::tuple<TextureFiltering, TextureFiltering, bool>{
		TextureFiltering::LINEAR_MIPMAP_LINEAR, TextureFiltering::LINEAR, true};

	ResourcePtr<Texture> texture = Create(realPath, min, mag, mipmap);
	if (texture) {
		texture->path = path;
	}
	return texture;
}

ResourcePtr<Texture> TextureLoader::CreateFromFileFloat(const std::string& path) {
	std::string realPath = getRealPath(path);

	auto [min, mag, mipmap] = std::tuple<TextureFiltering, TextureFiltering, bool>{
		TextureFiltering::LINEAR, TextureFiltering::LINEAR, false};

	ResourcePtr<Texture> texture = CreateFloat(realPath, min, mag, mipmap);
	if (texture) {
		texture->path = path;
	}
	return texture;
}

ResourcePtr<Texture> TextureLoader::createColor(const std::string& name, uint8_t r, uint8_t g, uint8_t b, TextureFiltering firstFilter, TextureFiltering secondFilter, bool generateMipmap) {
	if (auto resource = getResource<Texture>(name)) {
		return resource;
	}
	else {
		auto newResource = CreateColor(r, g, b, firstFilter, secondFilter, generateMipmap);
		if (newResource) {
			return registerResource(name, newResource);
		}
		else {
			return nullptr;
		}
	}
}

ResourcePtr<Texture> TextureLoader::CreateEmpty(uint32_t width, uint32_t height, bool isFloating, int channels, TextureFormat format) {
	auto res = std::make_shared<Texture>();
	res->path = "";
	res->width = width;
	res->height = height;
	res->textureType = GL_TEXTURE_2D;
	res->format = format;
	
	GLenum type = isFloating ? GL_FLOAT : GL_UNSIGNED_BYTE;
	
	GLenum dataChannels = GL_RGB;
	switch (channels) {
	case 1:
		dataChannels = GL_RED;
		break;
	case 2:
		dataChannels = GL_RG;
		break;
	case 3:
		dataChannels = GL_RGB;
		break;
	case 4:
		dataChannels = GL_RGBA;
		break;
	default:
		break;
	}
	
	glBindTexture(GL_TEXTURE_2D, res->getId());
	glTexImage2D(GL_TEXTURE_2D, 0, res->formatTable[(int)format],
		(GLsizei)width, (GLsizei)height, 0, dataChannels, type, nullptr);

	res->generateMipmaps();
	return res;
}


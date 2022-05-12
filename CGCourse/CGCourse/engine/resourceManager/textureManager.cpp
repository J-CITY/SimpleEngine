#include "textureManager.h"
#include "resource/texture.h"
#define STB_IMAGE_IMPLEMENTATION
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

std::shared_ptr<Texture> TextureLoader::Create(const std::string& filepath, TextureFiltering firstFilter, TextureFiltering secondFilter, bool generateMipmap) {
	GLuint textureID;
	int textureWidth;
	int textureHeight;
	int bitsPerPixel;
	glGenTextures(1, &textureID);

	stbi_set_flip_vertically_on_load(true);
	unsigned char* dataBuffer = stbi_load(filepath.c_str(), &textureWidth, &textureHeight, &bitsPerPixel, 4);

	if (dataBuffer) {
		glBindTexture(GL_TEXTURE_2D, textureID);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, textureWidth, textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, dataBuffer);

		if (generateMipmap) {
			glGenerateMipmap(GL_TEXTURE_2D);
		}

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, static_cast<GLint>(firstFilter));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, static_cast<GLint>(secondFilter));

		stbi_image_free(dataBuffer);
		glBindTexture(GL_TEXTURE_2D, 0);

		return std::make_shared<Texture>(filepath, textureID, textureWidth, textureHeight, bitsPerPixel, firstFilter, secondFilter, generateMipmap);
	}
	else {
		stbi_image_free(dataBuffer);
		glBindTexture(GL_TEXTURE_2D, 0);
		return nullptr;
	}
}

std::shared_ptr<Texture> TextureLoader::CreateColor(uint8_t r, uint8_t g, uint8_t b, TextureFiltering firstFilter, TextureFiltering secondFilter, bool generateMipmap) {
	uint8_t buffer [] = {r, g, b};

	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &buffer);

	if (generateMipmap) {
		glGenerateMipmap(GL_TEXTURE_2D);
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, static_cast<GLint>(firstFilter));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, static_cast<GLint>(secondFilter));

	glBindTexture(GL_TEXTURE_2D, 0);

	return std::make_shared<Texture>("", textureID, 1, 1, 32, firstFilter, secondFilter, generateMipmap);
}

std::shared_ptr<Texture> TextureLoader::CreateColor(uint32_t p_data, TextureFiltering firstFilter, TextureFiltering secondFilter, bool generateMipmap) {
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &p_data);

	if (generateMipmap) {
		glGenerateMipmap(GL_TEXTURE_2D);
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, static_cast<GLint>(firstFilter));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, static_cast<GLint>(secondFilter));

	glBindTexture(GL_TEXTURE_2D, 0);

	return std::make_shared<Texture>("", textureID, 1, 1, 32, firstFilter, secondFilter, generateMipmap);
}

std::shared_ptr<CubeMap> TextureLoader::CreateColorCM(uint8_t r, uint8_t g, uint8_t b) {
	auto cubemap = std::make_shared<CubeMap>();
	uint8_t buffer [] = {r, g, b};
	std::array<uint8_t*, 6> sides = {buffer, buffer, buffer, buffer, buffer, buffer};
	cubemap->Load(sides, 1, 1);
	return cubemap;
}

std::shared_ptr<Texture> TextureLoader::CreateFromMemory(uint8_t* data, uint32_t width, uint32_t height, TextureFiltering firstFilter, TextureFiltering secondFilter, bool generateMipmap) {
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

	return std::make_shared<Texture>("", textureID, 1, 1, 32, firstFilter, secondFilter, generateMipmap);
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

void TextureLoader::Reload(Texture& texture, const std::string& filePath, TextureFiltering firstFilter, TextureFiltering secondFilter, bool generateMipmap) {
	std::shared_ptr<Texture> newTexture = Create(filePath, firstFilter, secondFilter, generateMipmap);

	if (newTexture) {
		glDeleteTextures(1, &texture.id);

		*const_cast<uint32_t*>(&texture.id) = newTexture->id;
		*const_cast<uint32_t*>(&texture.width) = newTexture->width;
		*const_cast<uint32_t*>(&texture.height) = newTexture->height;
		*const_cast<uint32_t*>(&texture.bitsPerPixel) = newTexture->bitsPerPixel;
		*const_cast<TextureFiltering*>(&texture.firstFilter) = newTexture->firstFilter;
		*const_cast<TextureFiltering*>(&texture.secondFilter) = newTexture->secondFilter;
		*const_cast<bool*>(&texture.isMimapped) = newTexture->isMimapped;
	}
}

void TextureLoader::Destroy(std::shared_ptr<Texture> textureInstance) {
	if (textureInstance) {
		glDeleteTextures(1, &textureInstance->id);
		textureInstance.reset();
		textureInstance = nullptr;
	}
}

std::shared_ptr<Texture> TextureLoader::createResource(const std::string& path) {
	std::string realPath = getRealPath(path);

	auto [min, mag, mipmap] = std::tuple<TextureFiltering, TextureFiltering, bool>{
		TextureFiltering::LINEAR_MIPMAP_LINEAR, TextureFiltering::LINEAR, true};

	std::shared_ptr<Texture> texture = Create(realPath, min, mag, mipmap);
	if (texture) {
		texture->path = path;
	}
	return texture;
}
void TextureLoader::destroyResource(std::shared_ptr<Texture> res) {
	Destroy(res);
}


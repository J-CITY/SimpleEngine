#include "texture.h"

#include <stdexcept>

using namespace KUMA;
using namespace KUMA::RESOURCES;


Texture::Texture() {
	glGenTextures(1, &id);
}
Texture::Texture(const std::string path, uint32_t id, uint32_t width, uint32_t height, uint32_t bpp, TextureFiltering firstFilter, TextureFiltering secondFilter, bool generateMipmap) :
	path(path),
	id(id),
	width(width),
	height(height),
	bitsPerPixel(bpp),
	firstFilter(firstFilter),
	secondFilter(secondFilter),
	isMimapped(generateMipmap) {
}

//Texture::Texture(Texture& texture) noexcept {
//	this->width = texture.width;
//	this->height = texture.height;
//	this->textureType = texture.textureType;
//	this->path = std::move(texture.path);
//	//this->samples = texture.samples;
//	this->format = texture.format;
//	this->id = texture.id;
//
//	//texture.id = 0;
//	////texture.activeId = 0;
//	//texture.width = 0;
//	//texture.height = 0;
//	//texture.path = "[[deleted]]";
//	////texture.samples = 0;
//}

Texture::Texture(unsigned w, unsigned h) noexcept {
	this->width = w;
	this->height = h;
	this->textureType = GL_TEXTURE_2D;
	this->path = "";
	this->format = TextureFormat::RG16F;

	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w, h, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  //используем режим GL_CLAMP_TO_EDGE, т.к. в противном случае фильтр размытия производил бы выборку повторяющихся значений текстуры!
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

Texture::~Texture() {
	glDeleteTextures(1, &id);
}

void Texture::bind(uint32_t slot) const {
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, id);
	boundId = slot;
}

void Texture::bindWithoutAttach() const {
	glBindTexture(GL_TEXTURE_2D, id);
}


void Texture::unbind() const {
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::generateMipmaps() const {
	bind(0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);
}

void Texture::loadDepth(int _width, int _height) {
	textureType = GL_TEXTURE_2D;

	glBindTexture(GL_TEXTURE_2D, id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, _width, _height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor [] = {1.0, 1.0, 1.0, 1.0};
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
}

void Texture::setFilter(TextureFiltering minFilter, TextureFiltering magFilter) {
	bindWithoutAttach();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, static_cast<GLint>(minFilter));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, static_cast<GLint>(magFilter));
}

void Texture::setWrapType(TextureWrap wrapS, TextureWrap wrapT) {
	bindWithoutAttach();
	glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, &wrapTable[(int)wrapS]);
	glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, &wrapTable[(int)wrapT]);
}

void Texture::CopyTexture(const Texture& from, const Texture& to) {
	if (from.width != to.width || from.height != to.height) {
		throw std::logic_error("Textures have different size");
	}
	glCopyImageSubData(from.id, GL_TEXTURE_2D, 0, 0, 0, 0,
		to.id, GL_TEXTURE_2D, 0, 0, 0, 0,
		from.width, from.height, 1);
}

void Texture::load(uint8_t* data, int width, int height, int channels, bool isFloating, TextureFormat format) {
	this->path = "raw";
	this->width = width;
	this->height = height;
	this->textureType = GL_TEXTURE_2D;
	this->format = format;
			
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
			
	glBindTexture(GL_TEXTURE_2D, id);
	glTexImage2D(GL_TEXTURE_2D, 0, formatTable[(int)this->format],
	             (GLsizei)width, (GLsizei)height, 0, dataChannels, type, data);
			
	generateMipmaps();
}

uint32_t Texture::getId() const {
	return id;
}

uint32_t Texture::getWidth() const {
	return width;
}

uint32_t Texture::getHeight() const {
	return height;
}

uint32_t Texture::getType() const {
	return textureType;
}

std::string Texture::getPath() const {
	return path;
}

CubeMap::CubeMap() {
	glGenTextures(1, &id);
}

CubeMap::CubeMap(const std::string path): path(path) {
	glGenTextures(1, &id);
}

CubeMap::CubeMap(CubeMap& other) noexcept {
	this->id = other.id;
	//this->id = other.id;
	this->width = other.width;
	this->height = other.height;
	this->channels = other.channels;

	//other.id = 0;
	////other.active = 0;
	//other.width = 0;
	//other.height = 0;
	//other.channels = 0;
}

uint32_t CubeMap::getWidth() const {
	return width;
}

uint32_t CubeMap::getHeight() const {
	return height;
}

void CubeMap::bind(uint32_t slot) const {
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_CUBE_MAP, id);
	BoundId = slot;
}

void CubeMap::bindWithoutAttach() const {
	glBindTexture(GL_TEXTURE_CUBE_MAP, id);
}

void CubeMap::unbind() const {
	glBindTexture(GL_TEXTURE_2D, 0);
}

void CubeMap::LoadDepth(int width, int height) {
	this->width = width;
	this->height = height;
	this->path = "depth";
	this->channels = 1;

	glBindTexture(GL_TEXTURE_CUBE_MAP, id);

	for (unsigned int i = 0; i < 6; ++i)
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	/*for (size_t i = 0; i < 6; i++) {
					glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + (GLenum)i, 0, GL_DEPTH_COMPONENT,
						width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
				}

				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);

				float border [] = {1.0f, 1.0f, 1.0f, 1.0f};
				glTexParameterfv(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BORDER_COLOR, border);

				GenerateMipmaps();*/
}

void CubeMap::generateMipmaps() const {
	bindWithoutAttach();
	//glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	//glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
}

void CubeMap::Load(const std::array<uint8_t*, 6>& data, size_t width, size_t height) {
	this->width = width;
	this->height = height;
	this->channels = 3;
	this->path = "raw";
	glBindTexture(GL_TEXTURE_CUBE_MAP, id);
	for (size_t i = 0; i < data.size(); i++) {
		glTexImage2D(types[i], 0, GL_RGB,
		             (GLsizei)width, (GLsizei)height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data[i]);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	generateMipmaps();
}

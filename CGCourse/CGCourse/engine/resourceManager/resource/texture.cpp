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

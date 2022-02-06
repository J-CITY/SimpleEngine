#include "TextureRT.h"


ConstantTexture::ConstantTexture() {}

ConstantTexture::ConstantTexture(KUMA::MATHGL::Vector3& _color)
	: color(_color) {

}

KUMA::MATHGL::Vector3 ConstantTexture::Value(const KUMA::MATHGL::Vector2& uv, const KUMA::MATHGL::Vector3& p) const {
	return color;
}
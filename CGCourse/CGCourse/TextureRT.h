#pragma once
#include "engine/utils/math/Vector3.h"

namespace KUMA {
	namespace MATHGL {
		struct Vector2;
	}
}

class Texture {
public:
	virtual KUMA::MATHGL::Vector3 Value(const KUMA::MATHGL::Vector2& uv, const KUMA::MATHGL::Vector3& p) const = 0;
};

class ConstantTexture : public Texture {
public:
	ConstantTexture();
	ConstantTexture(KUMA::MATHGL::Vector3& _color);

	KUMA::MATHGL::Vector3 Value(const KUMA::MATHGL::Vector2& uv, const KUMA::MATHGL::Vector3& p) const override;

private:
	KUMA::MATHGL::Vector3 color;
};

#include "vector2.h"

#include <cmath>
#include <stdexcept>

using namespace KUMA;
using namespace KUMA::MATHGL;

float& Vector2::operator[](const int index) { 
	if (index > 1) {
		throw std::logic_error("Index > 3");
	}
	switch (index) {
	case 0: return x;
	case 1: return y;
	}
}
const float& Vector2::operator[](const int index) const { 
	if (index > 1) {
		throw std::logic_error("Index > 3");
	}
	switch (index) {
	case 0: return x;
	case 1: return y;
	}
}

Vector2& Vector2::operator=(const Vector2& right) {
	if (this == &right) {
		return *this;
	}
	x = right.x;
	y = right.y;
	return *this;
}

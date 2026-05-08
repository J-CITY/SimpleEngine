#include "animation.h"

namespace IKIGAI::SKELETON {
	MATH::Vector3f translationDelta(const MATH::Vector3f& reference, MATH::Vector3f additive) {
		return additive - reference;
	}

	MATH::Vector3f scaleDelta(const MATH::Vector3f& reference, MATH::Vector3f additive) {
		return additive / reference;
	}

	MATH::QuaternionF rotationDelta(const MATH::QuaternionF& reference, MATH::QuaternionF additive) {
		return MATH::QuaternionF::Conjugate(reference) * additive;
	}

	std::string trimmedName(const std::string& name) {
		size_t pos = name.find_first_of(':');
		if (pos != std::string::npos) {
			return name.substr(pos + 1);
		}
		return name;
	}
}

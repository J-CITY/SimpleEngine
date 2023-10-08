#include "boundingSphere.h"
#include "cmath"

float IKIGAI::RENDER::BoundingSphere::calcGrowth(const BoundingSphere& sphere) const {
	BoundingSphere newSphere;
	newSphere.createFromTwo(*this, sphere);

	float radiusDiff = (newSphere.radius - radius);

	// ( ( 4 / 3 ) * pi * r^3 )
	return 1.33333f * 3.1415 * radiusDiff * radiusDiff * radiusDiff;
}

float IKIGAI::RENDER::BoundingSphere::getSize() const {
	return 1.33333f * 3.1415 * radius * radius * radius; /* ( ( 4 / 3 ) * pi * r^3 ) */
}

int IKIGAI::RENDER::BoundingSphere::intersect(const BoundingSphere& sphere) const {
	float distance = MATHGL::Vector3::LengthSqrt(position - sphere.position);

	return (distance < (radius + sphere.radius)* (radius + sphere.radius)) ? 1 : 0;
}

void IKIGAI::RENDER::BoundingSphere::createFromTwo(const BoundingSphere& one, const BoundingSphere& two) {
	MATHGL::Vector3 centerOffset = two.position - one.position;
	float distance = centerOffset.LengthSqrt(centerOffset);

	float radiusDiff = two.radius - one.radius;

	if (radiusDiff * radiusDiff >= distance) {
		if (one.radius > two.radius) {
			position = one.position;
			radius = one.radius;
		}
		else {
			position = two.position;
			radius = two.radius;
		}
	}
	else {
		distance = sqrtf(distance);
		radius = (distance + one.radius + two.radius) * 0.5f;

		position = one.position;
		if (distance > 0) {
			position += centerOffset * ((radius - one.radius) / distance);
		}
	}
}

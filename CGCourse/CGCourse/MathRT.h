#pragma once
#include <random>

import glmath;

inline float Dot(const KUMA::MATHGL::Vector3& v1, const KUMA::MATHGL::Vector3& v2) {
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

class Ray;
class AABB {
public:
	// minimum and maximum extent of the box
	KUMA::MATHGL::Vector3 min, max;

	AABB();
	// a -> top back left of box, b -> bottom front right of box
	AABB(const KUMA::MATHGL::Vector3& a, const KUMA::MATHGL::Vector3& b);

	bool Hit(const Ray& ray, float& tMin, float& tMax) const;
	uint8_t GetLongestAxis() const;
	KUMA::MATHGL::Vector3 GetCentroidPoint() const;
	AABB ExpandBoundingBox(const AABB& box);
	AABB GetSurroundingBox(const AABB& box0, const AABB& box1);
};


/*
	p(t) = A + t * B
		where:	p - 3D pos along ray
				A - Ray Origin
				B - Ray Direction
				t - Ray parameter (any point on the line defined by the ray's origin and direction)
	@note(Darren): When B is normalized to unit length, t corresponds to the actual distance from start point A
*/
class Ray {
public:
	Ray() {}
	Ray(const KUMA::MATHGL::Vector3& _origin, const KUMA::MATHGL::Vector3& _direction, float ti = 0.0)
		: origin(_origin), direction(_direction), time(ti) {
	}

	KUMA::MATHGL::Vector3 PointAtParamater(float t) const { return origin + direction*t; }

	KUMA::MATHGL::Vector3 origin, direction;
	float time;
};

float randF(float start, float end);
KUMA::MATHGL::Vector3 RandomInUnitSphere();
KUMA::MATHGL::Vector3 RandomInUnitDisk();
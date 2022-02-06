#include "MathRT.h"


AABB::AABB() {}

AABB::AABB(const KUMA::MATHGL::Vector3& a, const KUMA::MATHGL::Vector3& b)
	: min(a), max(b) {

}

bool AABB::Hit(const Ray& ray, float& tMin, float& tMax) const {
	for (int a = 0; a < 3; a++) {
		float t0, t1;
		if (a == 0) {
			t0 = fminf((min.x - ray.origin.x) / ray.direction.x,
				(max.x - ray.origin.x) / ray.direction.x);
			t1 = fmaxf((min.x - ray.origin.x) / ray.direction.x,
				(max.x - ray.origin.x) / ray.direction.x);
		}
		else if (a == 1) {
			t0 = fminf((min.y - ray.origin.y) / ray.direction.y,
				(max.y - ray.origin.y) / ray.direction.y);
			t1 = fmaxf((min.y - ray.origin.y) / ray.direction.y,
				(max.y - ray.origin.y) / ray.direction.y);
		}
		else
		{
			if (a == 0) {
				t0 = fminf((min.z - ray.origin.z) / ray.direction.z,
					(max.z - ray.origin.z) / ray.direction.z);
				t1 = fmaxf((min.z - ray.origin.z) / ray.direction.z,
					(max.z - ray.origin.z) / ray.direction.z);
			}
		}
		tMin = fmaxf(t0, tMin);
		tMax = fminf(t1, tMax);

		if (tMax <= tMin)
			return false;
	}

	return true;
}

uint8_t AABB::GetLongestAxis() const {
	KUMA::MATHGL::Vector3 vec = max - min;

	if (vec.x > vec.y && vec.x > vec.z)	return 0;
	if (vec.y > vec.x && vec.y > vec.z)	return 1;
	if (vec.z > vec.x && vec.z > vec.y)	return 2;

	return 0;
}

KUMA::MATHGL::Vector3 AABB::GetCentroidPoint() const {
	return KUMA::MATHGL::Vector3( min *0.5f+ max*0.5f);
}

AABB AABB::ExpandBoundingBox(const AABB& box) {
	KUMA::MATHGL::Vector3 min(fminf(this->min.x, box.min.x), fminf(this->min.y, box.min.y), fminf(this->min.z, box.min.z));
	KUMA::MATHGL::Vector3 max(fmaxf(this->max.x, box.max.x), fmaxf(this->max.y, box.max.y), fmaxf(this->max.z, box.max.z));

	return AABB(min, max);
}

AABB AABB::GetSurroundingBox(const AABB& box0, const AABB& box1) {
	KUMA::MATHGL::Vector3 min(fminf(box0.min.x, box1.min.x), fminf(box0.min.y, box1.min.y), fminf(box0.min.z, box1.min.z));
	KUMA::MATHGL::Vector3 max(fmaxf(box0.max.x, box1.max.x), fmaxf(box0.max.y, box1.max.y), fmaxf(box0.max.z, box1.max.z));

	return AABB(min, max);
}


float randF(float start, float end) {
	static std::random_device rand_dev;
	static std::mt19937 generator(rand_dev());
	static std::uniform_real<float> distr(start, end);
	return distr(generator);
}

KUMA::MATHGL::Vector3 RandomInUnitSphere() {
	KUMA::MATHGL::Vector3 point;

	do {
		point = KUMA::MATHGL::Vector3(randF(-1.0f, 1.0f) * 2.0f, randF(-1.0f, 1.0f), randF(-1.0f, 1.0f)) - KUMA::MATHGL::Vector3(1.0f, 1.0f, 1.0f);
	} while (KUMA::MATHGL::Vector3::LengthSqrt(point) >= 1.0f);

	return point;
}

KUMA::MATHGL::Vector3 RandomInUnitDisk() {
	KUMA::MATHGL::Vector3 point;

	do {
		point = KUMA::MATHGL::Vector3(randF(-1.0f, 1.0f) * 2.0f, randF(-1.0f, 1.0f), 0.0f) - KUMA::MATHGL::Vector3(1.0f, 1.0f, 0.0f);
	} while (Dot(point, point) >= 1.0f);

	return point;
}

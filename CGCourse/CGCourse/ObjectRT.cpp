#include "ObjectRT.h"



#include "DiffuseLightRT.h"
#include "MathRT.h"
#include "SceneRT.h"
#include "engine/utils/math/Quaternion.h"
#include "engine/utils/math/Vector3.h"


#define CULLING true

Triangle::Triangle() {}

Triangle::Triangle(KUMA::MATHGL::Vector3& _p1, KUMA::MATHGL::Vector3& _p2, KUMA::MATHGL::Vector3& _p3)
	: p1(_p1), p2(_p2), p3(_p3) {

}

// Möller–Trumbore intersection algorithm
bool Triangle::Hit(const Ray& ray, float tMin, float tMax, HitRecord& hitRecord) const {
	KUMA::MATHGL::Vector3 d1 = p2 - p1;
	KUMA::MATHGL::Vector3 d2 = p3 - p1;
	KUMA::MATHGL::Vector3 pvec = KUMA::MATHGL::Vector3::Cross(ray.direction, d2);
	float det = Dot(d1, pvec);

	if (det < FLT_EPSILON)
		return false;

	float invDet = 1 / det;

	KUMA::MATHGL::Vector3 tvec = ray.origin - p1;
	float u = Dot(tvec, pvec) * invDet;

	if (u < 0 || u > 1)
		return false;

	KUMA::MATHGL::Vector3 qvec = KUMA::MATHGL::Vector3::Cross(tvec, d1);
	float v = Dot(ray.direction, qvec) * invDet;

	if (v < 0 || u + v > 1)
		return false;

	float t = Dot(d2, qvec) * invDet;

	if (t < tMin || t > tMax)
		return false;

	hitRecord.t = t;
	hitRecord.point = ray.PointAtParamater(hitRecord.t);
	hitRecord.normal = KUMA::MATHGL::Vector3::Cross(d1, d2);

	return true;
}

bool Triangle::BoundingBox(float t0, float t1, AABB& box) const {
	KUMA::MATHGL::Vector3 minExtent(fminf(fminf(p1.x, p2.x), p3.x), fminf(fminf(p1.y, p2.y), p3.y), fminf(fminf(p1.z, p2.z), p3.z));
	KUMA::MATHGL::Vector3 maxExtent(fmaxf(fmaxf(p1.x, p2.x), p3.x), fmaxf(fmaxf(p1.y, p2.y), p3.y), fmaxf(fmaxf(p1.z, p2.z), p3.z));

	box = AABB(minExtent, maxExtent);

	return true;
}

KUMA::MATHGL::Vector3 Triangle::Centroid() const {
	return (p1 + p2 + p3) / 3;
}


Sphere::Sphere(const KUMA::MATHGL::Vector3& _startPos, const float& _radius, Material* _material)
	: startPos(_startPos), radius(_radius), material(_material), endPos(_startPos), time0(0.0f), time1(0.0f), motionBlur(false) {

}

Sphere::Sphere(const KUMA::MATHGL::Vector3& _startPos, const KUMA::MATHGL::Vector3& _endPos, float _time0, float _time1,
	const float& _radius, Material* _material)
	: startPos(_startPos), endPos(_endPos), time0(_time0), time1(_time1), radius(_radius), material(_material), motionBlur(true) {

}

Sphere::~Sphere() {
	if (material)
		delete material;
}

bool Sphere::Hit(const Ray& ray, float tMin, float tMax, HitRecord& hitRecord) const {
	KUMA::MATHGL::Vector3 center = Center(ray.time);
	KUMA::MATHGL::Vector3 m = ray.origin - center;
	// @note(Darren): The ray direction is not normalized
	// Dot product of a vector itself is the square length of that vector
	float a = Dot(ray.direction, ray.direction);
	float b = Dot(m, ray.direction);
	float c = Dot(m, m) - (radius * radius);
	float discriminant = b * b - a * c;

	if (discriminant > 0) {
		float temp = (-b - sqrtf(discriminant)) / a;

		if (temp < tMax && temp > tMin) {
			hitRecord.t = temp;
			hitRecord.point = ray.PointAtParamater(hitRecord.t);
			hitRecord.normal = (hitRecord.point - Center(ray.time)) / radius;
			hitRecord.uv = SphereUV(hitRecord.normal);
			hitRecord.material = material;
			return true;
		}

		temp = (-b + sqrtf(discriminant)) / a;

		if (temp < tMax && temp > tMin) {
			hitRecord.t = temp;
			hitRecord.point = ray.PointAtParamater(hitRecord.t);
			hitRecord.normal = (hitRecord.point - Center(ray.time)) / radius;
			hitRecord.uv = SphereUV(hitRecord.normal);
			hitRecord.material = material;
			return true;
		}
	}

	return false;
}

KUMA::MATHGL::Vector2 Sphere::SphereUV(const KUMA::MATHGL::Vector3& p) const {
	float theta = asinf(p.y);
	float phi = atan2f(p.z, p.x);
	float u = 1 - (phi + PI) / (2 * PI);
	float v = (theta + PI / 2) / PI;

	return KUMA::MATHGL::Vector2(u, v);
}

/*
	Time is from 0-1 as set up in scene
	Time between pos of sphere is also 0-1
*/
KUMA::MATHGL::Vector3 Sphere::Center(float time) const {
	if (!motionBlur)
		return startPos;
	else
		//				  |<--------Range from 0-1-------->|
		return startPos + (endPos - startPos) * ((time - time0) / (time1 - time0));
}

bool Sphere::BoundingBox(float t0, float t1, AABB& box) const {
	if (!motionBlur)
		box = AABB(startPos - KUMA::MATHGL::Vector3(radius, radius, radius), startPos + KUMA::MATHGL::Vector3(radius, radius, radius));
	else {
		AABB box0(Center(t0) - KUMA::MATHGL::Vector3(radius, radius, radius), Center(t0) + KUMA::MATHGL::Vector3(radius, radius, radius));
		AABB box1(Center(t1) - KUMA::MATHGL::Vector3(radius, radius, radius), Center(t1) + KUMA::MATHGL::Vector3(radius, radius, radius));
		box = box.GetSurroundingBox(box0, box1);
	}

	return true;
}


Plane::Plane(KUMA::MATHGL::Vector3& _position, KUMA::MATHGL::Vector3& _normal, Material* _material)
	: position(_position), normal(_normal / KUMA::MATHGL::Vector3::Length(_normal)), material(_material) {

}

Plane::~Plane() {
	if (material)
		delete material;
}

bool Plane::Hit(const Ray& ray, float tMin, float tMax, HitRecord& hitRecord) const {
	float d = Dot(normal, ray.direction);

	if (fabs(d) < 0.001f)
		return false;

	KUMA::MATHGL::Vector3 a = position - ray.origin;
	float t = Dot(a, normal) / d;
	if (t < 0.001f)
		return false;

	hitRecord.t = t;
	hitRecord.normal = normal;
	hitRecord.point = ray.PointAtParamater(t);
	hitRecord.material = material;

	return true;
}

bool Plane::BoundingBox(float t0, float t1, AABB& box) const {
	box = AABB(KUMA::MATHGL::Vector3(-INFINITY, -INFINITY, -INFINITY), KUMA::MATHGL::Vector3(INFINITY, INFINITY, INFINITY));

	return true;
}

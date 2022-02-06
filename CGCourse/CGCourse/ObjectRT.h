#pragma once
#include "SceneRT.h"

class Triangle {
public:
	Triangle();
	Triangle(KUMA::MATHGL::Vector3& _p1, KUMA::MATHGL::Vector3& _p2, KUMA::MATHGL::Vector3& _p3);

	bool Hit(const Ray& ray, float tMin, float tMax, HitRecord& hitRecord) const;
	bool BoundingBox(float t0, float t1, AABB& box) const;
	KUMA::MATHGL::Vector3 Centroid() const;

private:
	KUMA::MATHGL::Vector3 p1, p2, p3;
};


class Sphere : public Hitable {
public:
	Sphere(const KUMA::MATHGL::Vector3& startPos, const KUMA::MATHGL::Vector3& endPos, float time0, float time1,
		const float& _radius, Material* material);
	Sphere(const KUMA::MATHGL::Vector3& position, const float& _radius, Material* material);
	~Sphere();

	bool Hit(const Ray& ray, float tMin, float tMax, HitRecord& hitRecord) const override;
	bool BoundingBox(float t0, float t1, AABB& box) const override;

private:
	KUMA::MATHGL::Vector2 SphereUV(const KUMA::MATHGL::Vector3& p) const;
	KUMA::MATHGL::Vector3 Center(float time) const;

	float radius;
	// For moition blur, static objects will have the same start/end pos and time
	KUMA::MATHGL::Vector3 startPos, endPos;
	float time0, time1;
	bool motionBlur;
	Material* material;
};

class Plane : public Hitable {
public:
	Plane(KUMA::MATHGL::Vector3& position, KUMA::MATHGL::Vector3& normal, Material* material);
	~Plane();

	bool Hit(const Ray& ray, float tMin, float tMax, HitRecord& hitRecord) const override;
	bool BoundingBox(float t0, float t1, AABB& box) const override;

private:
	KUMA::MATHGL::Vector3 position;
	KUMA::MATHGL::Vector3 normal;
	Material* material;
};

#pragma once
#include <vector>

#include "engine/utils/math/Vector2.h"
#include "engine/utils/math/Vector3.h"

class AABB;
class Ray;

class Material;

struct HitRecord {
	KUMA::MATHGL::Vector3 point;
	KUMA::MATHGL::Vector3 normal;
	KUMA::MATHGL::Vector2f uv;
	float t = 0.0f;
	Material* material = nullptr;
};

class Hitable {
public:
	virtual bool Hit(const Ray& ray, float tMin, float tMax, HitRecord& hitRecord) const = 0;
	virtual bool BoundingBox(float t0, float t1, AABB& box) const = 0;
};



class HitableList : public Hitable {
public:
	HitableList();
	~HitableList();

	void Add(Hitable* hitable);

	bool Hit(const Ray& ray, float tMin, float tMax, HitRecord& rec) const override;
	bool BoundingBox(float t0, float t1, AABB& box) const override;

private:
	std::vector<Hitable*> hitables;
};

struct SceneRT {
	HitableList sceneObects;
	void Add(Hitable* hitable) {
		sceneObects.Add(hitable);
	}
};

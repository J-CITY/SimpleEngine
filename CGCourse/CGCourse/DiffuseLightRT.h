#pragma once
#include "MathRT.h"
#include "SceneRT.h"
#include "TextureRT.h"
#include "engine/utils/math/Vector2.h"

struct HitRecord;
class Ray;

KUMA::MATHGL::Vector3 Reflect(const KUMA::MATHGL::Vector3& v, const KUMA::MATHGL::Vector3& n);
namespace KUMA {
	namespace MATHGL {
		//struct Vector2f;
		struct Vector3;
	}
}
class Material {
public:
	virtual bool Scatter(const Ray& rayIn, const HitRecord& hitRecord, KUMA::MATHGL::Vector3& attenuation, Ray& scattered) const = 0;
	virtual KUMA::MATHGL::Vector3 Emitted(KUMA::MATHGL::Vector2f uv, const KUMA::MATHGL::Vector3& p) const { return KUMA::MATHGL::Vector3(); };
};

class Dielectric : public Material {
public:
	Dielectric(float ri);

	bool Scatter(const Ray& rayIn, const HitRecord& hitRecord, KUMA::MATHGL::Vector3& attenuation, Ray& scattered) const override {
		KUMA::MATHGL::Vector3 outwardNormal;
		KUMA::MATHGL::Vector3 reflected = Reflect(rayIn.direction, hitRecord.normal);
		float ni_over_nt;
		attenuation = KUMA::MATHGL::Vector3(1.0f, 1.0f, 1.0f);
		KUMA::MATHGL::Vector3 refracted;
		float reflectProb;
		float cosine;

		if (Dot(rayIn.direction, hitRecord.normal) > 0) {
			outwardNormal = -hitRecord.normal;
			ni_over_nt = refractiveIndex;
			cosine = refractiveIndex * Dot(rayIn.direction, hitRecord.normal) / KUMA::MATHGL::Vector3::Length(rayIn.direction);
		}
		else {
			outwardNormal = hitRecord.normal;
			ni_over_nt = 1.0f / refractiveIndex;
			cosine = -Dot(rayIn.direction, hitRecord.normal) / KUMA::MATHGL::Vector3::Length(rayIn.direction);
		}

		if (Refract(rayIn.direction, outwardNormal, ni_over_nt, refracted)) {
			reflectProb = Schlick(cosine, refractiveIndex);
		}
		else {
			scattered = Ray(hitRecord.point, reflected);
			reflectProb = 1.0f;
		}

		if (randF(0.0f, 1.0f) < reflectProb) {
			scattered = Ray(hitRecord.point, reflected);
		}
		else {
			scattered = Ray(hitRecord.point, refracted);
		}

		return true;
	}

private:
	bool Refract(const KUMA::MATHGL::Vector3& v, const KUMA::MATHGL::Vector3& n, float ni_over_nt, KUMA::MATHGL::Vector3& refracted) const {
		KUMA::MATHGL::Vector3 uv = (v) / KUMA::MATHGL::Vector3::Length(v);
		float dt = Dot(uv, n);
		float discriminant = 1.0f - (ni_over_nt * ni_over_nt) * (1 - dt * dt);

		if (discriminant > 0) {
			refracted = (uv - n* dt)* ni_over_nt - n * sqrt(discriminant);
			return true;
		}
		else
			return false;
	}

	float Schlick(float cosine, float refractiveIndex) const {
		float r0 = (1 - refractiveIndex) / (1 + refractiveIndex);
		r0 = r0 * r0;

		return r0 + (1 - r0) * pow((1 - cosine), 5);
	}

	float refractiveIndex;
};

class Lambertian : public Material {
public:
	Lambertian(Texture* _albedo) : albedo(_albedo){}
	~Lambertian() {
		delete albedo;
	}


	bool Scatter(const Ray& rayIn, const HitRecord& hitRecord, KUMA::MATHGL::Vector3& attenuation, Ray& scattered) const override {
		KUMA::MATHGL::Vector3 target = hitRecord.point + hitRecord.normal + RandomInUnitSphere();
		scattered = Ray(hitRecord.point, target - hitRecord.point, rayIn.time);
		attenuation = albedo->Value(hitRecord.uv, hitRecord.point);

		return true;
	}

private:
	Texture* albedo;
};

class Metal : public Material {
public:
	Metal(const KUMA::MATHGL::Vector3& _albedo, const float _roughness = 0.0f) : albedo(_albedo), roughness(_roughness){};

	bool Scatter(const Ray& rayIn, const HitRecord& hitRecord, KUMA::MATHGL::Vector3& attenuation, Ray& scattered) const override {
		KUMA::MATHGL::Vector3 reflected = Reflect((rayIn.direction)/ KUMA::MATHGL::Vector3::Length(rayIn.direction), hitRecord.normal);
		scattered = Ray(hitRecord.point, reflected + RandomInUnitSphere()*roughness);
		attenuation = albedo;

		return (Dot(scattered.direction, hitRecord.normal) > 0);
	}

private:
	KUMA::MATHGL::Vector3 albedo;
	float roughness;
};


class DiffuseLight : public Material {
public:
	DiffuseLight(Texture* texture): emitTexture(texture){};

	bool Scatter(const Ray& rayIn, 
		const HitRecord& hitRecord, 
		KUMA::MATHGL::Vector3& attenuation, Ray& scattered) const override {
		return false;
	}

	KUMA::MATHGL::Vector3 Emitted(KUMA::MATHGL::Vector2f uv,
		const KUMA::MATHGL::Vector3& p) const override {
		return emitTexture->Value(uv, p);
	};

private:
	Texture* emitTexture;
};

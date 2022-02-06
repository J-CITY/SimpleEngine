#include "CameraRT.h"

#include "MathRT.h"
#include "engine/utils/math/Quaternion.h"


CameraRT::CameraRT() {}

CameraRT::CameraRT(KUMA::MATHGL::Vector3& position, 
	KUMA::MATHGL::Vector3& lookAt, 
	KUMA::MATHGL::Vector3& upVec, float vfov, float aspectRatio,
	float aperture, float focusDistance, 
	float _shutterOpenTime, float _shutterCloseTime) {
	shutterOpenTime = _shutterOpenTime;
	shutterCloseTime = _shutterCloseTime;
	lensRadius = aperture / 2;

	origin = position;
	auto v = (position - lookAt);
	w = v / KUMA::MATHGL::Vector3::Length(v);
	auto cross = KUMA::MATHGL::Vector3::Cross(upVec, w);
	u = (cross) / KUMA::MATHGL::Vector3::Length(cross);
	v = KUMA::MATHGL::Vector3::Cross(w, u);

	float theta = vfov * PI / 180.0f;
	float halfHeight = tan(theta / 2);
	float halfWidth = aspectRatio * halfHeight;

	lowerLeftCorner = origin - u*halfWidth * focusDistance 
		- v*halfHeight * focusDistance - w*focusDistance ;
	horizontal = u*2 * halfWidth * focusDistance;
	vertical = v * 2 * halfHeight * focusDistance;
}

void CameraRT::CalculateRay(Ray& ray, float s, float t) {
	auto rd = RandomInUnitDisk() * lensRadius;
	auto offset = u * rd.x + v * rd.y;
	float time = shutterOpenTime + randF(0.0f, 1.0f) * (shutterCloseTime - shutterOpenTime);

	// @brett_refactor(darren): Can not longer access ray variables because of forward declartion.
	ray.origin = origin + offset;
	ray.direction = lowerLeftCorner + horizontal *s+  vertical *t- origin - offset;
	ray.time = time;
}
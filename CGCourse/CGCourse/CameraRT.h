#pragma once

#include "engine/utils/math/Vector3.h"

class Ray;

class CameraRT {
public:
	CameraRT();
	CameraRT(KUMA::MATHGL::Vector3& position, 
		KUMA::MATHGL::Vector3& lookAt, 
		KUMA::MATHGL::Vector3& upVec, 
		float vfov, float aspectRatio,
		float aperture, float focusDistance, 
		float shutterOpenTime, float shutterCloseTime);

	void CalculateRay(Ray& ray, float s, float t);

private:
	KUMA::MATHGL::Vector3 origin;
	KUMA::MATHGL::Vector3 lowerLeftCorner;
	KUMA::MATHGL::Vector3 horizontal;
	KUMA::MATHGL::Vector3 vertical;
	KUMA::MATHGL::Vector3 u, v, w;
	float lensRadius;
	float shutterOpenTime, shutterCloseTime;
};

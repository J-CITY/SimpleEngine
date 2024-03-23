#include "imguiWidgets.h"

#include "IconsFontAwesome5.h"
#include "imgui.h"
#include "imgui_internal.h"

#include "misc/cpp/imgui_stdlib.h"
#include "utilsModule/assertion.h"

bool IKIGAI::IMGUI::InputText(const std::string& name, const std::string& tag, std::string& text) {
	ImGui::Text(name.c_str());
	ImGui::SameLine();
	return ImGui::InputText(tag.c_str(), &text);
}

bool IKIGAI::IMGUI::drawFloatN(std::string& name, int size, float* vec, float step, float mn, float mx) {
	switch (size) {
	case 2:
		return ImGui::DragFloat2(name.c_str(), vec, step);
	case 3:
		return ImGui::DragFloat3(name.c_str(), vec, step);
	case 4:
		return ImGui::DragFloat4(name.c_str(), vec, step);
	default: ASSERT("Error");
	}
	return false;
}

bool IKIGAI::IMGUI::drawColorN(std::string& name, int size, float* vec) {
	switch (size) {
	case 3:
		return ImGui::ColorEdit3(name.c_str(), vec);
	case 4:
		return ImGui::ColorEdit4(name.c_str(), vec);
	default: ASSERT("Error");
	}
	return false;
}

bool IKIGAI::IMGUI::SliderFloatWithSteps(const char* label, float* v, float v_min, float v_max, float v_step, const char* display_format) {
	if (!display_format)
		display_format = "%.3f";

	char text_buf[64] = {};
	ImFormatString(text_buf, IM_ARRAYSIZE(text_buf), display_format, *v);

	// Map from [v_min,v_max] to [0,N]
	const int countValues = int((v_max - v_min) / v_step);
	int v_i = int((*v - v_min) / v_step);
	const bool value_changed = ImGui::SliderInt(label, &v_i, 0, countValues, text_buf);

	// Remap from [0,N] to [v_min,v_max]
	*v = v_min + float(v_i) * v_step;
	return value_changed;
}

IKIGAI::MATH::Vector3f IKIGAI::IMGUI::ProjectPointOntoRay(const MATH::Vector3f& rayOrigin,
	const MATH::Vector3f& rayDirection, const MATH::Vector3f& point)
{
	MATH::Vector3f originToPoint = point - rayOrigin;

	// assume the direction is not normalized
	float dist = (rayDirection.dot(originToPoint)) / MATH::Vector3f::Length(rayDirection);

	MATH::Vector3f result = rayDirection * dist;
	result = rayOrigin + result;

	return result;
}

bool IKIGAI::IMGUI::RaySphereIntersection(const MATH::Vector3f& rayOrigin, const MATH::Vector3f& rayDirection,
	const MATH::Vector3f& sphereCenter, float radius, std::vector<MATH::Vector3f>& hits)
{
	// make sure the direction is a unit vector
	MATH::Vector3f direction = MATH::Vector3f::Normalize(rayDirection);

	MATH::Vector3f originToCenter = sphereCenter - rayOrigin;

	// check whether the center of the sphere is behind the ray origin
	if (MATH::Vector3f::Dot(originToCenter, direction) < 0.0f) {
		// the sphere center is behind the ray -> intersection is only possible if the ray is within the sphere

		float distance = MATH::Vector3f::Length(originToCenter);
		if (distance > radius) {
			// ray origin is outside the sphere
			return false;
		} else if (distance > (radius - 0.000001f) && distance < (radius + 0.000001f)) {
			// ray origin is on the sphere
			hits.push_back(rayOrigin);
			return true;
		} else {
			// get the projection point from the sphere center onto the ray
			MATH::Vector3f projected = ProjectPointOntoRay(rayOrigin, direction, sphereCenter);

			// get the intersection point
			float lengthProjCenter = MATH::Vector3f::Length(projected - sphereCenter);
			float dist = sqrtf((radius * radius) + (lengthProjCenter * lengthProjCenter));

			float lengthOriginIntersection = dist - MATH::Vector3f::Length(projected - rayOrigin);

			MATH::Vector3f hit = rayOrigin + (direction * lengthOriginIntersection);
			hits.push_back(hit);

			return true;
		}

	} else {
		// the sphere center is in front of the ray

		MATH::Vector3f projected = ProjectPointOntoRay(rayOrigin, direction, sphereCenter);

		float lengthProjCenter = MATH::Vector3f::Length(sphereCenter - projected);
		if (lengthProjCenter > radius) {
			// the projection point is outside the sphere -> no intersection
			return false;
		} else if (lengthProjCenter > (radius - 0.000001f) && lengthProjCenter < (radius + 0.000001f)) {
			// the projection point is on the sphere
			hits.push_back(projected);
			return true;
		}

		float lengthProjIntersection1 = sqrtf((radius * radius) + (lengthProjCenter * lengthProjCenter));

		// check whether the ray origin is within the sphere
		if (MATH::Vector3f::Length(originToCenter) < radius) {
			// there is only one intersection
			float lengthOriginIntersection = MATH::Vector3f::Length(projected - rayOrigin) + lengthProjIntersection1;

			MATH::Vector3f hit = rayOrigin + (direction * lengthOriginIntersection);
			hits.push_back(hit);

			return true;
		} else {
			// there are two intersections
			// get the first intersection
			float lengthProjOrigin = MATH::Vector3f::Length(projected - rayOrigin);
			float lengthOriginIntersection = lengthProjOrigin - lengthProjIntersection1;
			MATH::Vector3f hit = rayOrigin + (direction * lengthOriginIntersection);
			hits.push_back(hit);

			// get the second intersection point
			lengthOriginIntersection = lengthProjOrigin + lengthProjIntersection1;
			hit = rayOrigin + (direction * lengthOriginIntersection);
			hits.push_back(hit);

			return true;
		}

	}
}

IKIGAI::IMGUI::CombineVecEdit::CombineVecEdit(std::string_view name, size_t vecSize, MODE mode, float mn, float mx,
                                              float step):
	mVecSize(vecSize), mName(name), mMode(mode), mMin(mn), mMax(mx), mStep(step)
{
	switch (mVecSize) {
	case 2: mButtonLblVec = "XY"; mButtonLblColor = "RG"; mMode = MODE::POS; break;
	case 3: mButtonLblVec = "XYZ"; mButtonLblColor = "RGB"; mWidth = 150.0f; break;
	case 4: mButtonLblVec = "XYZW"; mButtonLblColor = "RGBA"; mWidth = 150.0f; break;
	default: ASSERT("Support only 2, 3, 4 vec size");
	}
	if (mode == MODE::POS) {
		mIsColorMode = false;
	}
	if (mode == MODE::COLOR) {
		mIsColorMode = true;
	}
}

bool IKIGAI::IMGUI::CombineVecEdit::draw(float* vec) {
	ImGui::Columns(2);
	ImGui::SetColumnWidth(1, 200.0f);

	ImGui::Text(mName.c_str());
	ImGui::NextColumn();

	ImGui::SetNextItemWidth(mWidth);

	bool res = false;
	auto _nameId = "##" + mName;
	if (mIsColorMode) {
		if (IKIGAI::IMGUI::drawColorN(_nameId, mVecSize, vec)) {
			res = true;
		}
	} else {
		if (IKIGAI::IMGUI::drawFloatN(_nameId, mVecSize, vec, mStep, mMin, mMax)) {
			res = true;
		}
	}
	if (mMode == MODE::COMBINE) {
		ImGui::SameLine();
		ImGui::SetNextItemWidth(50);
		if (ImGui::SmallButton(mIsColorMode ? mButtonLblColor.c_str() : mButtonLblVec.c_str())) {
			mIsColorMode = !mIsColorMode;
		}
	}

	ImGui::NextColumn();
	ImGui::Columns(1);
	return res;
}

#pragma once
#include <vector>

#include "imgui.h"
#include "mathModule/math.h"
#include "utilsModule/assertion.h"

namespace IKIGAI::IMGUI {
	bool InputText(const std::string& name, const std::string& tag, std::string& text);


	bool drawFloatN(std::string& name, int size, float* vec, float step = 0.1f, float mn = 0.0f, float mx = 0.0f);

	bool drawColorN(std::string& name, int size, float* vec);

	bool SliderFloatWithSteps(const char* label, float* v, float v_min, float v_max, float v_step, const char* display_format = "%.3f");


	MATH::Vector3f ProjectPointOntoRay(const MATH::Vector3f& rayOrigin, const MATH::Vector3f& rayDirection, const MATH::Vector3f& point);

	bool RaySphereIntersection(const MATH::Vector3f& rayOrigin, const MATH::Vector3f& rayDirection, const MATH::Vector3f& sphereCenter, float radius, std::vector<MATH::Vector3f>& hits);


	class CombineVecEdit {
		size_t mVecSize = 3;
		std::string mName;
		bool mIsColorMode = false;
		std::string mButtonLblColor;
		std::string mButtonLblVec;
		float mWidth = 200.0f;

		float mMin = 0.0f;
		float mMax = 0.0f;
		float mStep = 0.1f;
	public:
		enum class MODE {
			COMBINE, COLOR, POS
		};
		MODE mMode = MODE::COMBINE;
		CombineVecEdit(std::string_view name, size_t vecSize, MODE mode = MODE::COMBINE, float mn = 0.0f, float mx = 0.0f, float step = 0.1f);
		inline static std::unordered_map<std::string, CombineVecEdit> Data;

		//add type check
		template<class T>
		bool draw(T& vec) {
			return draw(reinterpret_cast<float*>(&vec));
		}
	private:
		bool draw(float* vec);
	};

	class ScalarEdit {
		std::string mName;
	public:
	
		inline static std::unordered_map<std::string, ScalarEdit> Data;
		enum class TYPE {
			INT, FLOAT, BOOL
		};
		TYPE mType;
		ScalarEdit(std::string_view name, TYPE type): mType(type), mName(name) {
			
		}
		template<class T>
		bool draw(T& val) {
			bool res = false;
			ImGui::Columns(2);
			ImGui::SetColumnWidth(1, 200.0f);
	
			ImGui::Text(mName.c_str());
			ImGui::NextColumn();
	
			ImGui::SetNextItemWidth(200.0f);
	
			const auto _nameId = "##" + mName;
	
			res = draw(_nameId, val);
	
			ImGui::NextColumn();
			ImGui::Columns(1);
			return res;
		}
	private:
		template<class T>
		bool draw(const std::string& _nameId, T& val) {
			return false;
		}
		template<>
		bool draw<int>(const std::string& _nameId, int& val) {
			return ImGui::DragInt(_nameId.c_str(), &val);
		}
		template<>
		bool draw<float>(const std::string& _nameId, float& val) {
			return ImGui::DragFloat(_nameId.c_str(), &val, 0.1f);
		}
		template<>
		bool draw<bool>(const std::string& _nameId, bool& val) {
			return ImGui::Checkbox(_nameId.c_str(), &val);
		}
	};
}

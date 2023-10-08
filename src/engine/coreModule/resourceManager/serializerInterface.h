#pragma once

#include <nlohmann/json.hpp>
import glmath;

namespace IKIGAI {
	namespace RESOURCES {
		inline void SerializeVec4(nlohmann::json& j, const MATHGL::Vector4& vec) {
			j = {vec.x, vec.y, vec.z, vec.w};
		}
		inline void DeserializeVec4(const nlohmann::json& j, MATHGL::Vector4& vec) {
			vec.x = j[0];
			vec.y = j[1];
			vec.z = j[2];
			vec.w = j[3];
		}
		inline MATHGL::Vector4 DeserializeVec4(const nlohmann::json& j) {
			MATHGL::Vector4 vec;
			vec.x = j[0];
			vec.y = j[1];
			vec.z = j[2];
			vec.w = j[3];
			return vec;
		}
		
		inline void SerializeVec3(nlohmann::json& j, const MATHGL::Vector3& vec) {
			j = { vec.x, vec.y, vec.z };
		}
		inline void DeserializeVec3(const nlohmann::json& j, MATHGL::Vector3& vec) {
			vec.x = j[0];
			vec.y = j[1];
			vec.z = j[2];
		}
		inline MATHGL::Vector3 DeserializeVec3(const nlohmann::json& j) {
			MATHGL::Vector3 vec;
			vec.x = j[0];
			vec.y = j[1];
			vec.z = j[2];
			return vec;
		}

		inline void SerializeVec2(nlohmann::json& j, const MATHGL::Vector2f& vec) {
			j = {vec.x, vec.y};
		}
		inline void DeserializeVec2(const nlohmann::json& j, MATHGL::Vector2f& vec) {
			vec.x = j[0];
			vec.y = j[1];
		}
		inline MATHGL::Vector2f DeserializeVec2(const nlohmann::json& j) {
			MATHGL::Vector2f vec;
			vec.x = j[0];
			vec.y = j[1];
			return vec;
		}

		inline void SerializeQuat(nlohmann::json& j, const MATHGL::Quaternion& in) {
			j = {in.x, in.y, in.z, in.w};
		}
		inline void DeserializeQuat(const nlohmann::json& j, MATHGL::Quaternion& in) {
			in.x = j[0];
			in.y = j[1];
			in.z = j[2];
			in.w = j[3];
		}
		inline MATHGL::Quaternion DeserializeQuat(const nlohmann::json& j) {
			MATHGL::Quaternion in;
			in.x = j[0];
			in.y = j[1];
			in.z = j[2];
			in.w = j[3];
			return in;
		}

		
		class Serializable {
		public:
			Serializable() = default;
			virtual ~Serializable() = default;
			virtual void onSerialize(nlohmann::json& j) = 0;
			virtual void onDeserialize(nlohmann::json& j) = 0;
		};
	}
}

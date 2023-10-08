#pragma once

#include <string>
#include <rttr/type>
#include "../loader.h"
#include <nlohmann/json.hpp>
#include <result.h>

namespace IKIGAI
{
	namespace UTILS
	{
		struct JsonError;
		struct JsonOk;
	}
}


namespace IKIGAI::UTILS
{

	Result<JsonOk, IKIGAI::UTILS::JsonError> fromJsonRecursively(rttr::instance obj, nlohmann::json& json_object);

	struct JsonError {
		enum class Kind {
			PARSE,
			FILE_NOT_EXIST
		};
		JsonError(Kind kind, std::string text): kind(kind), text(text) {}
	//private:
		Kind kind;
		std::string text;
	};

	struct JsonOk {};

	std::string ToJsonStr(rttr::instance obj);
	nlohmann::json ToJson(rttr::instance obj);


	static std::string readFileIntoString(const std::string& path) {
		std::ifstream input_file(path);
		if (!input_file.is_open()) {
			std::cerr << "Could not open the file - '" << path << "'" << std::endl;
			exit(EXIT_FAILURE);
		}
		return std::string((std::istreambuf_iterator<char>(input_file)), std::istreambuf_iterator<char>());
	}

	template<class T>
	Result<T, JsonError> FromJsonStr(const std::string& jsonStr) {
		nlohmann::json data;
		try {
			data = nlohmann::json::parse(jsonStr, nullptr, true, true);
		}
		catch (const std::exception& e) {
			return Err(JsonError(JsonError::Kind::PARSE, e.what()));
		}
		
		T obj;
		const Result<JsonOk, JsonError> res = fromJsonRecursively(obj, data);

		if (res.isErr()) {
			return Err(JsonError(res.unwrapErr()));
		}
		return Ok(obj);
	}

	template<class T>
	Result<T, JsonError> FromJson(nlohmann::json& data) {
		auto v = rttr::type::get<T>().create();
		T obj;
		const Result<JsonOk, JsonError> res = fromJsonRecursively(obj, data);

		if (res.isErr()) {
			return Err(JsonError(res.unwrapErr()));
		}
		return Ok(obj);
	}

	template<class T>
	Result<JsonOk, JsonError> FromJson(T& obj, nlohmann::json& data) {
		const Result<JsonOk, JsonError> res = fromJsonRecursively(obj, data);
		if (res.isErr()) {
			return Err(JsonError(res.unwrapErr()));
		}
		return Ok(JsonOk());
	}

	template<class T>
	Result<T, JsonError> FromJson(const std::string& path) {
		const std::string jsonData = readFileIntoString(UTILS::getRealPath(path));
		return FromJsonStr<T>(jsonData);

		//std::ifstream f(UTILS::getRealPath(path));
		//if (!f) {
		//	return Err(JsonError(JsonError::Kind::FILE_NOT_EXIST, path));
		//}
		//nlohmann::json data;
		//try {
		//	data = nlohmann::json::parse(f, nullptr, true, true);
		//}
		//catch (const std::exception& e) {
		//	f.close();
		//	return Err(JsonError(JsonError::Kind::PARSE, e.what()));
		//}
		//f.close();
		//
		//T obj;
		//const Result<int, JsonError> res = fromJsonRecursively(obj, data);
		//
		//if (res.isErr()) {
		//	return Err(JsonError(res.unwrapErr()));
		//}
		//return Ok(obj);
	}
}

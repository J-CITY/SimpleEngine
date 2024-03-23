#pragma once
#include <fstream>
#include <iostream>
#include <string>
#include <serdepp/serializer.hpp>
#include <nlohmann/json.hpp>
#include <serdepp/adaptor/nlohmann_json.hpp>
#include "pathGetter.h"
#include "result.h"

namespace IKIGAI::UTILS {

	struct JsonError {
		enum class Kind {
			PARSE,
			FILE_NOT_EXIST,
			SEREALIZE
		};
		JsonError() = default;
		JsonError(Kind kind, std::string&& text) : kind(kind), text(std::move(text)) {}
		Kind kind;
		std::string text;
	};

	struct JsonOk {};

	static Result<std::string, JsonError> ReadFileIntoString(const std::string& path) {
		std::ifstream input_file(path);
		if (!input_file.is_open()) {
			Err(JsonError(JsonError::Kind::FILE_NOT_EXIST, "Can not open file: " + path));
		}
		return Ok(std::string((std::istreambuf_iterator<char>(input_file)), std::istreambuf_iterator<char>()));
	}

	template<class T>
	Result<T, JsonError> FromJsonStr(const std::string& jsonStr) {
		nlohmann::json data;
		try {
			data = nlohmann::json::parse(jsonStr, nullptr, true, true);
		} catch (const std::exception& e) {
			return Err(JsonError(JsonError::Kind::PARSE, e.what()));
		}
		try {
			auto obj = serde::deserialize<T>(data);
			return Ok(obj);
		} catch (std::exception& e) {
			return Err(JsonError(JsonError::Kind::PARSE, e.what()));
		}
	}

	template<class T>
	Result<T, JsonError> FromJson(nlohmann::json& data) {
		try {
			auto obj = serde::deserialize<T>(data);
			return Ok(obj);
		} catch (std::exception& e) {
			return Err(JsonError(JsonError::Kind::PARSE, e.what()));
		}
	}

	template<class T>
	Result<JsonOk, JsonError> FromJson(T& obj, nlohmann::json& data) {
		try {
			obj = serde::deserialize<T>(data);
			return Ok(JsonOk());
		} catch (std::exception& e) {
			return Err(JsonError(JsonError::Kind::PARSE, e.what()));
		}
	}

	template<class T>
	Result<T, JsonError> FromJson(const std::string& path) {
		auto res = ReadFileIntoString(UTILS::GetRealPath(path));
		if (res.isErr()) {
			return Err(res.unwrapErr());
		}
		return FromJsonStr<T>(res.unwrap());
	}

	template<class T>
	Result<nlohmann::json, JsonError> ToJson(const T& obj) {
		try {
			nlohmann::json jsonData = serde::serialize<nlohmann::json>(obj);
			return Ok(jsonData);
		} catch (std::exception& e) {
			return Err(JsonError(JsonError::Kind::SEREALIZE, e.what()));
		}
	}

	template<class T>
	Result<std::string, JsonError> ToJsonStr(const T& obj, int tab=-1) {
		auto res = ToJson<T>(obj);
		if (res.isErr()) {
			return Err(res.unwrapErr());
		}
		return Ok(res.unwrap().dump(tab));
	}
}

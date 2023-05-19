#pragma once

#include <string>
#include <rttr/type>
#include "../loader.h"
#include "json.hpp"

namespace KUMA
{
	namespace UTILS
	{
		struct JsonError;
	}
}


namespace KUMA::UTILS
{

	Result<int, KUMA::UTILS::JsonError> fromJsonRecursively(rttr::instance obj, nlohmann::json& json_object);

	struct JsonError {
		enum class Kind {
			PARSE,
			FILE_NOT_EXIST
		};
		JsonError(Kind kind, std::string text): kind(kind), text(text) {}
	private:
		Kind kind;
		std::string text;
	};

	std::string ToJson(rttr::instance obj);


	template<class T>
	Result<T, JsonError> FromJson(const std::string& path) {
		std::ifstream f(UTILS::getRealPath(path));
		if (!f) {
			return Err(JsonError(JsonError::Kind::FILE_NOT_EXIST, path));
		}
		nlohmann::json data;
		try {
			data = nlohmann::json::parse(f, nullptr, true, true);
		}
		catch (const std::exception& e) {
			f.close();
			return Err(JsonError(JsonError::Kind::PARSE, e.what()));
		}
		f.close();

		rttr::instance obj;
		const Result<int, JsonError> res = fromJsonRecursively(obj, data);

		if (res.isErr()) {
			return Err(JsonError(res.unwrapErr()));
		}
		return Ok(obj);
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
		const Result<int, JsonError> res = fromJsonRecursively(obj, data);

		if (res.isErr()) {
			return Err(JsonError(res.unwrapErr()));
		}
		return Ok(obj);
	}
}

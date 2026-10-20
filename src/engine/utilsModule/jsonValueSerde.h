#pragma once
#include <serdepp/adaptor/nlohmann_json.hpp>
namespace serde {
// json is iterable, so serdepp otherwise recursively treats each scalar as a
// sequence of json values. A JSON-valued descriptor field is an identity copy.
template <> struct serde_adaptor<nlohmann::json, nlohmann::json, type::seq_t> {
	static void from(nlohmann::json& source, std::string_view key, nlohmann::json& value) {
		value = key.empty() ? source : source.at(std::string(key));
	}
	static void into(nlohmann::json& target, std::string_view key, const nlohmann::json& value) {
		if (key.empty())
			target = value;
		else
			target[std::string(key)] = value;
	}
};
}

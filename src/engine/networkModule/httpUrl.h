#pragma once
#include "httpClient.h"
#include "networkRuntime.h"
#include <algorithm>
#include <cctype>
namespace IKIGAI::NETWORK::DETAIL {
struct HttpUrl {
	std::string host, port, authority, target;
	bool tls;
	explicit HttpUrl(const HttpRequest& request, bool websocket = false) {
		const auto separator = request.url.find("://");
		if (separator == std::string::npos || request.url.find_first_of("\r\n\t ") != std::string::npos)
			throw NetworkError(NetworkErrorCode::invalidArgument, "Invalid URL");
		const auto scheme = request.url.substr(0, separator);
		tls = scheme == (websocket ? "wss" : "https");
		if (!tls && scheme != (websocket ? "ws" : "http")) throw NetworkError(NetworkErrorCode::invalidArgument, "Unsupported URL scheme");
		const auto begin = separator + 3;
		const auto end = request.url.find_first_of("/?#", begin);
		authority = request.url.substr(begin, end == std::string::npos ? end : end - begin);
		if (authority.empty() || authority.find('@') != std::string::npos) throw NetworkError(NetworkErrorCode::invalidArgument, "Invalid URL authority");
		if (authority.front() == '[') {
			const auto bracket = authority.find(']');
			if (bracket == std::string::npos) throw NetworkError(NetworkErrorCode::invalidArgument, "Invalid IPv6 authority");
			host = authority.substr(1, bracket - 1);
			if (bracket + 1 < authority.size()) {
				if (authority[bracket + 1] != ':') throw NetworkError(NetworkErrorCode::invalidArgument, "Invalid URL port");
				port = authority.substr(bracket + 2);
			}
		} else {
			const auto colon = authority.find(':');
			host = authority.substr(0, colon);
			if (colon != std::string::npos) port = authority.substr(colon + 1);
		}
		if (port.empty()) port = tls ? "443" : "80";
		if (port.size() > 5 || !std::all_of(port.begin(), port.end(), [](unsigned char c) { return c >= '0' && c <= '9'; })
			|| std::stoul(port) == 0 || std::stoul(port) > 65535 || host.empty())
			throw NetworkError(NetworkErrorCode::invalidArgument, "Invalid URL host/port");
		if (!tls && !(request.allowInsecureLocalhost && (host == "127.0.0.1" || host == "localhost" || host == "::1")))
			throw NetworkError(NetworkErrorCode::invalidArgument, "HTTPS required except for explicit loopback tests");
		target = end == std::string::npos ? "/" : request.url.substr(end);
		if (const auto fragment = target.find('#'); fragment != std::string::npos) target.resize(fragment);
		if (target.empty() || target.front() != '/') target.insert(target.begin(), '/');
	}
};
inline std::string LowerHeader(std::string value) {
	for (auto& c : value) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
	return value;
}
}

#include "networkAsioInternal.h"
#ifdef IKIGAI_NETWORK_ASIO
#include "httpUrl.h"
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#ifdef IKIGAI_NETWORK_TLS
#include <boost/asio/ssl.hpp>
#include <boost/beast/ssl.hpp>
#ifdef _WIN32
#include <wincrypt.h>
#include <windows.h>
#endif
#endif
#include <array>

namespace IKIGAI::NETWORK {
namespace {
	namespace asio = boost::asio;
	namespace beast = boost::beast;
	namespace http = beast::http;
	using Tcp = asio::ip::tcp;
	using namespace DETAIL;
	template <class Stream>
	asio::awaitable<void> ReadResponse(Stream& stream, const HttpRequest& request, const HttpUrl& url,
		const std::shared_ptr<TransferControl>& control, const std::function<void(const HttpResponse&)>& headers,
		const std::function<void(std::span<const uint8_t>)>& body) {
		http::request<http::empty_body> message { http::verb::get, url.target, 11 };
		message.set(http::field::host, url.authority);
		message.set(http::field::user_agent, "IkigaiEngine/1");
		message.keep_alive(false);
		for (const auto& [name, value] : request.headers) {
			if (name.empty() || name.find_first_of("\r\n: \t") != std::string::npos || value.find_first_of("\r\n") != std::string::npos)
				throw NetworkError(NetworkErrorCode::invalidArgument, "Invalid HTTP header");
			message.set(name, value);
		}
		co_await http::async_write(stream, message, asio::use_awaitable);
		beast::flat_buffer buffer;
		http::response_parser<http::buffer_body> parser;
		parser.body_limit(request.maxBodyBytes);
		parser.header_limit(65536);
		co_await http::async_read_header(stream, buffer, parser, asio::use_awaitable);
		HttpResponse response;
		response.status = parser.get().result_int();
		for (const auto& entry : parser.get().base())
			response.headers[LowerHeader(std::string(entry.name_string()))] = std::string(entry.value());
		headers(response);
		std::array<uint8_t, 65536> bytes;
		while (!parser.is_done()) {
			control->check();
			parser.get().body().data = bytes.data();
			parser.get().body().size = bytes.size();
			boost::system::error_code error;
			co_await http::async_read_some(stream, buffer, parser, asio::redirect_error(asio::use_awaitable, error));
			const auto size = bytes.size() - parser.get().body().size;
			if (size) {
				body({ bytes.data(), size });
				control->transferred += size;
			}
			if (error && error != http::error::need_buffer)
				throw boost::system::system_error(error);
		}
	}
#ifdef IKIGAI_NETWORK_TLS
}
namespace DETAIL {
	void ConfigureTrust(asio::ssl::context& context, const std::string& bundle) {
		if (!bundle.empty()) {
			context.load_verify_file(bundle);
			return;
		}
		context.set_default_verify_paths();
#ifdef _WIN32
		// OpenSSL does not automatically import the Windows system trust store.
		auto store = CertOpenSystemStoreW(0, L"ROOT");
		if (!store)
			throw NetworkError(NetworkErrorCode::tls, "Cannot open Windows root certificate store");
		PCCERT_CONTEXT certificate = nullptr;
		while ((certificate = CertEnumCertificatesInStore(store, certificate))) {
			const unsigned char* encoded = certificate->pbCertEncoded;
			if (auto* value = d2i_X509(nullptr, &encoded, certificate->cbCertEncoded)) {
				X509_STORE_add_cert(SSL_CTX_get_cert_store(context.native_handle()), value);
				X509_free(value);
			}
		}
		CertCloseStore(store, 0);
		ERR_clear_error();
#endif
	}
}
namespace {
#endif
	class BeastHttpClient final : public IHttpClient {
		std::shared_ptr<NetworkRuntime> mRuntime;

	public:
		explicit BeastHttpClient(std::shared_ptr<NetworkRuntime> runtime)
			: mRuntime(runtime ? std::move(runtime) : std::make_shared<NetworkRuntime>()) { }
		std::shared_future<void> getAsync(HttpRequest request, std::shared_ptr<TransferControl> control,
			std::function<void(const HttpResponse&)> headers, std::function<void(std::span<const uint8_t>)> body) override {
			const auto state = NativeAccess::state(mRuntime);
			try {
				HttpUrl url(request);
				if (!control || !headers || !body || request.timeoutSeconds <= 0)
					throw NetworkError(NetworkErrorCode::invalidArgument, "Invalid HTTP request");
				auto resolver = std::make_shared<Tcp::resolver>(state->io);
				const auto timeout = std::chrono::duration_cast<Timeout>(std::chrono::seconds(request.timeoutSeconds));
				if (!url.tls) {
					auto stream = std::make_shared<beast::tcp_stream>(state->io);
					return Start<void>(
						state, timeout,
						[stream, resolver, request = std::move(request), url, control, headers = std::move(headers),
							body = std::move(body)]() -> asio::awaitable<void> {
							auto endpoints = co_await resolver->async_resolve(url.host, url.port, asio::use_awaitable);
							co_await stream->async_connect(endpoints, asio::use_awaitable);
							co_await ReadResponse(*stream, request, url, control, headers, body);
						},
						[stream, resolver] {
							resolver->cancel();
							boost::system::error_code ignored;
							stream->socket().close(ignored);
						},
						control)
						.result;
				}
#ifdef IKIGAI_NETWORK_TLS
				auto context = std::make_shared<asio::ssl::context>(asio::ssl::context::tls_client);
				auto stream = std::make_shared<beast::ssl_stream<beast::tcp_stream>>(state->io, *context);
				stream->set_verify_mode(asio::ssl::verify_peer);
				stream->set_verify_callback(asio::ssl::host_name_verification(url.host));
				if (!SSL_set_tlsext_host_name(stream->native_handle(), url.host.c_str()))
					throw NetworkError(NetworkErrorCode::tls, "Cannot set TLS server name");
				return Start<void>(
					state, timeout,
					[stream, context, resolver, request = std::move(request), url, control, headers = std::move(headers),
						body = std::move(body)]() -> asio::awaitable<void> {
						ConfigureTrust(*context, request.caBundle);
						auto endpoints = co_await resolver->async_resolve(url.host, url.port, asio::use_awaitable);
						co_await beast::get_lowest_layer(*stream).async_connect(endpoints, asio::use_awaitable);
						co_await stream->async_handshake(asio::ssl::stream_base::client, asio::use_awaitable);
						co_await ReadResponse(*stream, request, url, control, headers, body);
						// Full HTTP framing was validated; close transport without a second blocking shutdown.
					},
					[stream, resolver] {
						resolver->cancel();
						boost::system::error_code ignored;
						beast::get_lowest_layer(*stream).socket().close(ignored);
					},
					control)
					.result;
#else
				throw NetworkError(NetworkErrorCode::unsupported, "This build has no TLS backend");
#endif
			} catch (...) {
				std::promise<void> promise;
				promise.set_exception(std::current_exception());
				return promise.get_future().share();
			}
		}
	};
}
std::unique_ptr<IHttpClient> CreateHttpClient(std::shared_ptr<NetworkRuntime> runtime) {
	return std::make_unique<BeastHttpClient>(std::move(runtime));
}
}
#endif

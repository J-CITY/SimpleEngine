#include "webSocket.h"
#include "networkAsioInternal.h"
#ifdef IKIGAI_NETWORK_ASIO
#include "httpUrl.h"
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#ifdef IKIGAI_NETWORK_TLS
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <boost/asio/ssl.hpp>
#endif
namespace IKIGAI::NETWORK {
namespace {
using namespace DETAIL;
namespace beast = boost::beast;
namespace ws = beast::websocket;
using Tcp = asio::ip::tcp;
using PlainWs = ws::stream<beast::tcp_stream>;
#ifdef IKIGAI_NETWORK_TLS
using SecureWs = ws::stream<beast::ssl_stream<beast::tcp_stream>>;
#endif
struct WsState {
	std::shared_ptr<AsioState> runtime;
	std::shared_ptr<PlainWs> plain;
#ifdef IKIGAI_NETWORK_TLS
	std::shared_ptr<asio::ssl::context> context;
	std::shared_ptr<SecureWs> secure;
#endif
	std::shared_ptr<Tcp::resolver> resolver;
	bool connected = false, connecting = false, reading = false, writing = false;
	explicit WsState(std::shared_ptr<AsioState> runtime) : runtime(std::move(runtime)), resolver(std::make_shared<Tcp::resolver>(this->runtime->io)) { }
	void abort() {
		resolver->cancel(); boost::system::error_code ignored;
		if (plain) beast::get_lowest_layer(*plain).socket().close(ignored);
#ifdef IKIGAI_NETWORK_TLS
		if (secure) beast::get_lowest_layer(*secure).socket().close(ignored);
#endif
		connected = false;
	}
};
struct WsBusy {
	bool& flag;
	explicit WsBusy(bool& flag) : flag(flag) { if (flag) throw NetworkError(NetworkErrorCode::invalidArgument, "WebSocket operation already pending"); flag = true; }
	~WsBusy() { flag = false; }
};
template<class S> asio::awaitable<WebSocketMessage> ReadWs(S& stream) {
	beast::flat_buffer buffer;
	co_await stream.async_read(buffer, asio::use_awaitable);
	WebSocketMessage message; message.text = stream.got_text(); message.bytes.resize(buffer.size());
	asio::buffer_copy(asio::buffer(message.bytes), buffer.data()); co_return message;
}
class BeastWebSocket final : public IWebSocketClient {
	std::shared_ptr<NetworkRuntime> mRuntime;
	std::shared_ptr<WsState> mState;
public:
	explicit BeastWebSocket(std::shared_ptr<NetworkRuntime> runtime) : mRuntime(runtime ? std::move(runtime) : std::make_shared<NetworkRuntime>()),
		mState(std::make_shared<WsState>(NativeAccess::state(mRuntime))) { }
	~BeastWebSocket() override { auto state = mState; state->runtime->post([state] { state->abort(); }); }
	Operation<void> connect(HttpRequest request) override {
		auto state = mState;
		const auto timeout = std::chrono::duration_cast<Timeout>(std::chrono::seconds(request.timeoutSeconds));
		return Start<void>(state->runtime, timeout, [state, request = std::move(request)]() -> asio::awaitable<void> {
			WsBusy busy(state->connecting);
			if (state->connected || state->reading || state->writing) throw NetworkError(NetworkErrorCode::invalidArgument, "WebSocket is already in use");
			HttpUrl url(request, true);
			if (!request.headers.empty()) throw NetworkError(NetworkErrorCode::invalidArgument, "Custom WebSocket headers are not portable to browsers");
			auto endpoints = co_await state->resolver->async_resolve(url.host, url.port, asio::use_awaitable);
			state->plain.reset();
#ifdef IKIGAI_NETWORK_TLS
			state->secure.reset();
#endif
			if (!url.tls) {
				state->plain = std::make_shared<PlainWs>(state->runtime->io);
				state->plain->read_message_max(1024 * 1024);
				co_await beast::get_lowest_layer(*state->plain).async_connect(endpoints, asio::use_awaitable);
				co_await state->plain->async_handshake(url.authority, url.target, asio::use_awaitable);
			} else {
#ifdef IKIGAI_NETWORK_TLS
				state->context = std::make_shared<asio::ssl::context>(asio::ssl::context::tls_client);
				ConfigureTrust(*state->context, request.caBundle);
				state->secure = std::make_shared<SecureWs>(state->runtime->io, *state->context);
				state->secure->read_message_max(1024 * 1024);
				state->secure->next_layer().set_verify_mode(asio::ssl::verify_peer);
				state->secure->next_layer().set_verify_callback(asio::ssl::host_name_verification(url.host));
				if (!SSL_set_tlsext_host_name(state->secure->next_layer().native_handle(), url.host.c_str())) throw NetworkError(NetworkErrorCode::tls, "Cannot set TLS server name");
				co_await beast::get_lowest_layer(*state->secure).async_connect(endpoints, asio::use_awaitable);
				co_await state->secure->next_layer().async_handshake(asio::ssl::stream_base::client, asio::use_awaitable);
				co_await state->secure->async_handshake(url.authority, url.target, asio::use_awaitable);
#else
				throw NetworkError(NetworkErrorCode::unsupported, "WSS requires a TLS backend");
#endif
			}
			state->connected = true;
		}, [state] { state->abort(); });
	}
	Operation<WebSocketMessage> read(Timeout timeout) override {
		auto state = mState;
		return Start<WebSocketMessage>(state->runtime, timeout, [state]() -> asio::awaitable<WebSocketMessage> {
			if (!state->connected) throw NetworkError(NetworkErrorCode::io, "WebSocket is not connected");
			WsBusy busy(state->reading);
			if (state->plain) co_return co_await ReadWs(*state->plain);
#ifdef IKIGAI_NETWORK_TLS
			co_return co_await ReadWs(*state->secure);
#else
			throw NetworkError(NetworkErrorCode::unsupported, "WSS unavailable");
#endif
		}, [state] { state->abort(); });
	}
	Operation<size_t> write(WebSocketMessage message, Timeout timeout) override {
		auto state = mState;
		return Start<size_t>(state->runtime, timeout, [state, message = std::move(message)]() -> asio::awaitable<size_t> {
			if (!state->connected || message.bytes.size() > 1024 * 1024) throw NetworkError(NetworkErrorCode::invalidArgument, "WebSocket is disconnected or message exceeds 1 MiB");
			WsBusy busy(state->writing);
			if (state->plain) { state->plain->text(message.text); co_return co_await state->plain->async_write(asio::buffer(message.bytes), asio::use_awaitable); }
#ifdef IKIGAI_NETWORK_TLS
			state->secure->text(message.text); co_return co_await state->secure->async_write(asio::buffer(message.bytes), asio::use_awaitable);
#else
			throw NetworkError(NetworkErrorCode::unsupported, "WSS unavailable");
#endif
		}, [state] { state->abort(); });
	}
	Operation<void> close(Timeout timeout) override {
		auto state = mState;
		return Start<void>(state->runtime, timeout, [state]() -> asio::awaitable<void> {
			if (!state->connected) co_return;
			WsBusy busy(state->writing);
			if (state->plain) co_await state->plain->async_close(ws::close_code::normal, asio::use_awaitable);
#ifdef IKIGAI_NETWORK_TLS
			else co_await state->secure->async_close(ws::close_code::normal, asio::use_awaitable);
#endif
			state->abort();
		}, [state] { state->abort(); });
	}
};
}
std::unique_ptr<IWebSocketClient> CreateWebSocketClient(std::shared_ptr<NetworkRuntime> runtime) { return std::make_unique<BeastWebSocket>(std::move(runtime)); }
}
#elif !defined(IKIGAI_NETWORK_WEB)
namespace IKIGAI::NETWORK {
namespace {
template<class T> Operation<T> WsUnavailable() {
	std::promise<T> promise; promise.set_exception(std::make_exception_ptr(NetworkError(NetworkErrorCode::unsupported, "WebSocket transport is disabled")));
	return {promise.get_future().share(), std::make_shared<TransferControl>()};
}
class OfflineWebSocket final : public IWebSocketClient {
	Operation<void> connect(HttpRequest) override { return WsUnavailable<void>(); }
	Operation<WebSocketMessage> read(Timeout) override { return WsUnavailable<WebSocketMessage>(); }
	Operation<size_t> write(WebSocketMessage, Timeout) override { return WsUnavailable<size_t>(); }
	Operation<void> close(Timeout) override { return WsUnavailable<void>(); }
};
}
std::unique_ptr<IWebSocketClient> CreateWebSocketClient(std::shared_ptr<NetworkRuntime>) { return std::make_unique<OfflineWebSocket>(); }
}
#endif

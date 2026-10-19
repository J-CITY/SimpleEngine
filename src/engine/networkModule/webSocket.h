#pragma once
#include "networkRuntime.h"
namespace IKIGAI::NETWORK {
struct WebSocketMessage {
	std::vector<uint8_t> bytes;
	bool text = false;
};
class IWebSocketClient {
public:
	virtual ~IWebSocketClient() = default;
	// URLs use wss://; ws:// is allowed only for explicit loopback tests.
	virtual Operation<void> connect(HttpRequest request) = 0;
	virtual Operation<WebSocketMessage> read(Timeout timeout = DefaultTimeout) = 0;
	virtual Operation<size_t> write(WebSocketMessage message, Timeout timeout = DefaultTimeout) = 0;
	virtual Operation<void> close(Timeout timeout = DefaultTimeout) = 0;
};
std::unique_ptr<IWebSocketClient> CreateWebSocketClient(std::shared_ptr<NetworkRuntime> runtime = { });
}

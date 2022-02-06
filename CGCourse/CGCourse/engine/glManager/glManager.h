#pragma once
#include <cstdint>

namespace KUMA::GL_SYSTEM {
	struct DriverSettings {
		bool debugMode = false;
	};
	class GlManager {
	public:
		GlManager(const DriverSettings& settings);
		~GlManager() = default;
	private:
		void initGlew();
		static void __stdcall GLDebugMessageCallback(uint32_t source, uint32_t type, uint32_t id, uint32_t severity, int32_t length, const char* message, const void* userParam);
	};
}

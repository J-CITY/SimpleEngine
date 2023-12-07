#pragma once
#include <chrono>

#include "core.h"
#include <utilsModule/pointers/objPtr.h>

#ifdef DX12_BACKEND
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#endif

namespace IKIGAI {
	namespace CORE_SYSTEM {
		class App {
		public:
#ifdef DX12_BACKEND
			inline static HINSTANCE hInstance;
#endif
			App();
			~App();
			void run();
			[[nodiscard]] bool isRunning() const;
			void preUpdate(std::chrono::duration<double> dt);
			void update(std::chrono::duration<double> dt);
			void postUpdate(std::chrono::duration<double> dt);
			Ref<Core> getCore();
		protected:
			Core core;
		};
	}
}

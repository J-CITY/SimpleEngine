#pragma once
#include <functional>

#include "Gamepad.h"

namespace KUMA::INPUT {
	class GamepadMgr {
	public:
		static GamepadMgr& Instance() {
			static GamepadMgr theGamepadMgr;
			return theGamepadMgr;
		}

		void initialize();
		void update(std::function<void(Gamepad::GamepadData&)> cb);
	private:
		GamepadMgr();
		~GamepadMgr() {}

		std::vector<Gamepad> gamepads;
	};
}
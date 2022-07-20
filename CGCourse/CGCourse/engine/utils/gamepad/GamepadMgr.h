#pragma once
#include <functional>

#include "Gamepad.h"

namespace KUMA::INPUT {
	class GamepadMgr {
	public:
		static GamepadMgr& Instance();

		void initialize();
		void update(std::function<void(Gamepad::GamepadData&)> cb);
	private:
		GamepadMgr();
		~GamepadMgr() = default;

		std::vector<Gamepad> gamepads;
	};
}
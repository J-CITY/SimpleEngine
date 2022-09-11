#pragma once
#include <functional>

#include "Gamepad.h"

namespace KUMA::INPUT {
	class GamepadManager {
	public:
		static GamepadManager& Instance();

		void initialize();
		void update(std::function<void(const Gamepad::GamepadData&)> cb);
	private:
		GamepadManager();
		~GamepadManager() = default;

		std::vector<Gamepad> gamepads;
	};
}
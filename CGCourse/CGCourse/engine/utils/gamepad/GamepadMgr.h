#pragma once
#include "Gamepad.h"

/// XInput
#include "Windows.h"
#include "Xinput.h"
#pragma comment(lib,"XInput.lib") // include libraries
#pragma comment(lib,"Xinput9_1_0.lib") // include libraries

/// files
#include <string>
#include <fstream>
#include <iostream>

class GamepadMgr
{
	public:
		static GamepadMgr& Instance() {
			static GamepadMgr theGamepadMgr;
			return theGamepadMgr;
		}

		void Initialize();

		Gamepad* GamepadOne() { return _gamepadOne; }
		Gamepad* GamepadTwo() { return _gamepadTwo; }
	private:
		GamepadMgr(); // ctor hidden
		GamepadMgr(GamepadMgr const&); // copy ctor hidden
		GamepadMgr& operator=(GamepadMgr const&) {} // assign op. hidden
		~GamepadMgr() {} // dtor hidden

		// Gamepads
		Gamepad* _gamepadOne;		
		Gamepad* _gamepadTwo;
};

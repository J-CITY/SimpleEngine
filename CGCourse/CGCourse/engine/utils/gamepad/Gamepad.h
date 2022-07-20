#pragma once

#include <map>
#include <string>

namespace KUMA::INPUT {
	class Gamepad {
	public:
		Gamepad() = delete;
		Gamepad(int number);
		~Gamepad() = default;

		enum class GAMEPAD_BUTTON {
			btn_a = 0,
			btn_b = 1,
			btn_x = 2,
			btn_y = 3,
			btn_leftStick = 4,
			btn_rightStick = 5,
			btn_back = 6,
			btn_start = 7,
			btn_lb = 8,
			btn_rb = 9,
			dpad_up = 11,
			dpad_down = 12,
			dpad_left = 13,
			dpad_right = 14
		};
		enum class GAMEPAD_AXIS {
			leftStick_X = 0,
			leftStick_Y = 1,
			rightStick_X = 2,
			rightStick_Y = 3
		};
		enum class GAMEPAD_TRIGGER {
			leftTrigger = 4,
			rightTrigger = 5
		};
		struct GamepadData {
			int id = 0;
			std::map<GAMEPAD_BUTTON, bool> buttons = {
				{ GAMEPAD_BUTTON::btn_a, false },
				{ GAMEPAD_BUTTON::btn_b, false },
				{ GAMEPAD_BUTTON::btn_x, false },
				{ GAMEPAD_BUTTON::btn_y, false },
				{ GAMEPAD_BUTTON::btn_leftStick, false },
				{ GAMEPAD_BUTTON::btn_rightStick, false },
				{ GAMEPAD_BUTTON::btn_back, false },
				{ GAMEPAD_BUTTON::btn_start, false },
				{ GAMEPAD_BUTTON::btn_lb, false },
				{ GAMEPAD_BUTTON::btn_rb, false },
				{ GAMEPAD_BUTTON::dpad_up, false },
				{ GAMEPAD_BUTTON::dpad_down, false },
				{ GAMEPAD_BUTTON::dpad_left, false },
				{ GAMEPAD_BUTTON::dpad_right, false }
			};
			float leftSticX = 0.0f;
			float leftSticY = 0.0f;
			float rightSticX = 0.0f;
			float rightSticY = 0.0f;
			float leftTrigger = 0.0f;
			float rightTrigger = 0.0f;
			bool operator!=(const GamepadData& gamepad_data) const;
		};
		bool isButtonPressed(GAMEPAD_BUTTON btn) const;
		float getAxisPosition(GAMEPAD_AXIS axis) const;
		float getTriggerValue(GAMEPAD_TRIGGER tgr) const;
		GamepadData getData() const;
		bool updateData();
	private:
		int gamepadNumber = 0;
		std::string name;
		GamepadData data;
	};
}

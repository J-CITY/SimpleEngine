#pragma once

#include <map>
#include <string>

namespace IKIGAI::INPUT {
	class Gamepad {
	public:
		Gamepad() = delete;
		Gamepad(int number, std::string name);
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
			int mId = 0;
			std::map<GAMEPAD_BUTTON, bool> mButtons = {
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
			float mLeftSticX = 0.0f;
			float mLeftSticY = 0.0f;
			float mRightSticX = 0.0f;
			float mRightSticY = 0.0f;
			float mLeftTrigger = 0.0f;
			float mRightTrigger = 0.0f;
			bool operator!=(const GamepadData& gamepad_data) const;
		};
		bool isButtonPressed(GAMEPAD_BUTTON btn) const;
		float getAxisPosition(GAMEPAD_AXIS axis) const;
		float getTriggerValue(GAMEPAD_TRIGGER tgr) const;
		const GamepadData& getData() const;
		GamepadData& getData();
		bool updateData();
		int getId() const;

	private:
		int mGamepadNumber = 0;
		std::string mName;
		GamepadData mData;
	};
}

#pragma once

#include "SFML\Window.hpp"

class Gamepad
{
	private:
		struct GamePadAxis {
			sf::Joystick::Axis Axis;
			bool inverted; // inverted: from 0 to -100
		};
		struct Trigger {
			bool isAxis;
			GamePadAxis Axis;
			int button;
		};
	public:
		Gamepad(int number, bool XInput);
		~Gamepad() {}

		enum GAMEPAD_BUTTON { btn_a, btn_b, btn_x, btn_y, btn_leftStick, btn_rightStick, btn_back, btn_start, btn_lb, btn_rb, dpad_up, dpad_down , dpad_left, dpad_right };
		enum GAMEPAD_AXIS { leftStick_X, leftStick_Y, rightStick_X, rightStick_Y };
		enum GAMEPAD_TRIGGER { leftTrigger, rightTrigger };
		
		bool isButtonPressed(GAMEPAD_BUTTON btn);
		float getAxisPosition(GAMEPAD_AXIS axis);
		float getTriggerValue(GAMEPAD_TRIGGER tgr);
	private:
		int gamepadNumber;
		bool isXInput;

		void LoadData(std::string line);
		bool assignButton(std::string n, std::string d);
		bool assignAxis(std::string n, std::string d);
		sf::Joystick::Axis extractAxis(int axisNumber);
	
		int getButtonNumber(GAMEPAD_BUTTON btn);
	
		float shrinkValue(float f, bool trigger);

		GamePadAxis LSTICK_X;
		GamePadAxis LSTICK_Y;
		int LSTICK;
		GamePadAxis RSTICK_X;
		GamePadAxis RSTICK_Y;
		int RSTICK;
		GamePadAxis POV_X;
		GamePadAxis POV_Y;
		Trigger LTRIGGER;
		Trigger RTRIGGER;
		int LB;
		int RB;
		int BACK;
		int START;
		int A;
		int B;
		int X;
		int Y;
};

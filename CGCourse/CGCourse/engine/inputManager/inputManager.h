#pragma once

#include <utility>

#include "../window/window.h"
#include "../utils/event.h"

namespace KUMA::INPUT_SYSTEM {
	enum class EKey {
		KEY_UNKNOWN = -1,
		KEY_SPACE = 32,
		KEY_APOSTROPHE = 39,
		KEY_COMMA = 44,
		KEY_MINUS = 45,
		KEY_PERIOD = 46,
		KEY_SLASH = 47,
		KEY_0 = 48,
		KEY_1 = 49,
		KEY_2 = 50,
		KEY_3 = 51,
		KEY_4 = 52,
		KEY_5 = 53,
		KEY_6 = 54,
		KEY_7 = 55,
		KEY_8 = 56,
		KEY_9 = 57,
		KEY_SEMICOLON = 59,
		KEY_EQUAL = 61,
		KEY_A = 0,
		KEY_B = 1,
		KEY_C = 2,
		KEY_D = 3,
		KEY_E = 4,
		KEY_F = 5,
		KEY_G = 6,
		KEY_H = 7,
		KEY_I = 8,
		KEY_J = 9,
		KEY_K = 10,
		KEY_L = 11,
		KEY_M = 12,
		KEY_N = 13,
		KEY_O = 14,
		KEY_P = 15,
		KEY_Q = 16,
		KEY_R = 17,
		KEY_S = 18,
		KEY_T = 19,
		KEY_U = 20,
		KEY_V = 21,
		KEY_W = 22,
		KEY_X = 23,
		KEY_Y = 24,
		KEY_Z = 25,
		KEY_LEFT_BRACKET = 91,
		KEY_BACKSLASH = 92,
		KEY_RIGHT_BRACKET = 93,
		KEY_GRAVE_ACCENT = 96,
		KEY_WORLD_1 = 61,
		KEY_WORLD_2 = 62,
		KEY_ESCAPE = 256,
		KEY_ENTER = 257,
		KEY_TAB = 258,
		KEY_BACKSPACE = 259,
		KEY_INSERT = 260,
		KEY_DELETE = 261,
		KEY_RIGHT = 262,
		KEY_LEFT = 263,
		KEY_DOWN = 264,
		KEY_UP = 265,
		KEY_PAGE_UP = 266,
		KEY_PAGE_DOWN = 267,
		KEY_HOME = 268,
		KEY_END = 269,
		KEY_CAPS_LOCK = 280,
		KEY_SCROLL_LOCK = 281,
		KEY_NUM_LOCK = 282,
		KEY_PRINT_SCREEN = 283,
		KEY_PAUSE = 284,
		KEY_F1 = 290,
		KEY_F2 = 291,
		KEY_F3 = 292,
		KEY_F4 = 293,
		KEY_F5 = 294,
		KEY_F6 = 295,
		KEY_F7 = 296,
		KEY_F8 = 297,
		KEY_F9 = 298,
		KEY_F10 = 299,
		KEY_F11 = 300,
		KEY_F12 = 301,
		KEY_F13 = 302,
		KEY_F14 = 303,
		KEY_F15 = 304,
		KEY_F16 = 305,
		KEY_F17 = 306,
		KEY_F18 = 307,
		KEY_F19 = 308,
		KEY_F20 = 309,
		KEY_F21 = 310,
		KEY_F22 = 311,
		KEY_F23 = 312,
		KEY_F24 = 313,
		KEY_F25 = 314,
		KEY_KP_0 = 320,
		KEY_KP_1 = 321,
		KEY_KP_2 = 322,
		KEY_KP_3 = 323,
		KEY_KP_4 = 324,
		KEY_KP_5 = 325,
		KEY_KP_6 = 326,
		KEY_KP_7 = 327,
		KEY_KP_8 = 328,
		KEY_KP_9 = 329,
		KEY_KP_DECIMAL = 330,
		KEY_KP_DIVIDE = 331,
		KEY_KP_MULTIPLY = 332,
		KEY_KP_SUBTRACT = 333,
		KEY_KP_ADD = 334,
		KEY_KP_ENTER = 335,
		KEY_KP_EQUAL = 336,
		KEY_LEFT_SHIFT = 340,
		KEY_LEFT_CONTROL = 341,
		KEY_LEFT_ALT = 342,
		KEY_LEFT_SUPER = 343,
		KEY_RIGHT_SHIFT = 344,
		KEY_RIGHT_CONTROL = 345,
		KEY_RIGHT_ALT = 346,
		KEY_RIGHT_SUPER = 347,
		KEY_MENU = 348
	};
	enum class EKeyState {
		KEY_UP = 0,
		KEY_DOWN = 1
	};
	enum class EMouseButton {
		MOUSE_BUTTON_1 = 0,
		MOUSE_BUTTON_2 = 1,
		MOUSE_BUTTON_3 = 2,
		MOUSE_BUTTON_4 = 3,
		MOUSE_BUTTON_5 = 4,
		MOUSE_BUTTON_6 = 5,
		MOUSE_BUTTON_7 = 6,
		MOUSE_BUTTON_8 = 7,
		MOUSE_BUTTON_LEFT = 0,
		MOUSE_BUTTON_RIGHT = 1,
		MOUSE_BUTTON_MIDDLE = 2
	};
	enum class EMouseButtonState {
		MOUSE_UP = 0,
		MOUSE_DOWN = 1
	};
	class InputManager {
	public:
		InputManager(WINDOW_SYSTEM::Window& p_window);
		~InputManager();

		bool isKeyPressed(EKey p_key) const;
		bool isKeyReleased(EKey p_key) const;
		bool isMouseButtonPressed(EMouseButton p_button) const;
		bool isMouseButtonReleased(EMouseButton p_button);

		std::pair<int, int> getMousePosition() const;
		void clearEvents();

	private:
		void onKeyPressed(int val);
		void onKeyReleased(int val);
		void onMouseButtonPressed(int val);
		void onMouseButtonReleased(int val);

	private:
		WINDOW_SYSTEM::Window& window;

		EVENT::Event<>::id keyPressedListener = EVENT::Event<>::id(0);
		EVENT::Event<>::id keyReleasedListener = EVENT::Event<>::id(0);
		EVENT::Event<>::id mouseButtonPressedListener = EVENT::Event<>::id(0);
		EVENT::Event<>::id mouseButtonReleasedListener = EVENT::Event<>::id(0);

		std::unordered_map<EKey, EKeyState> keyEvents;
		std::unordered_map<EMouseButton, EMouseButtonState>	mouseButtonEvents;
	};
}

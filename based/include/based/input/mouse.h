#pragma once

#include <array>

namespace based::input
{
	class Mouse
	{
	public:
		static void Initialize();
		static void Update();

		inline static int X() { return x; }
		inline static int Y() { return y; }

		inline static int DX() { return x - xLast; }
		inline static int DY() { return y - yLast; }

		static bool Button(int button);
		static bool ButtonDown(int button);
		static bool ButtonUp(int button);
	private:
		constexpr static const int ButtonCount = 5; //SDL supports up to 5 mouse buttons

		static int x, xLast;
		static int y, yLast;

		static std::array<bool, ButtonCount> buttons;
		static std::array<bool, ButtonCount> buttonsLast;
	};
}

// Adapted from SDL
enum
{
	BASED_INPUT_MOUSE_FIRST = 1,
	BASED_INPUT_MOUSE_LEFT = BASED_INPUT_MOUSE_FIRST,
	BASED_INPUT_MOUSE_MIDDLE = 2,
	BASED_INPUT_MOUSE_RIGHT = 3,
	BASED_INPUT_MOUSE_X1 = 4,
	BASED_INPUT_MOUSE_X2 = 5,
	BASED_INPUT_MOUSE_LAST = 5
};
#pragma once

#include <array>

#include "external/glm/glm.hpp"

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

		static void SetCursorLocked(bool locked);

		static glm::vec2 GetMousePosition();

	private:
		constexpr static int ButtonCount = 5; //SDL supports up to 5 mouse buttons

		static int x, xLast;
		static int y, yLast;

		inline static bool mCursorLocked = false;

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
#include "input/mouse.h"
#include "log.h"

#include <algorithm>

#include "SDL2/SDL_mouse.h"

namespace based::input
{
	int Mouse::x = 0;
	int Mouse::xLast = 0;
	int Mouse::y = 0;
	int Mouse::yLast = 0;

	std::array<bool, Mouse::ButtonCount> Mouse::buttons;
	std::array<bool, Mouse::ButtonCount> Mouse::buttonsLast;

	void Mouse::Initialize()
	{
		std::fill(buttons.begin(), buttons.end(), false);
		std::fill(buttonsLast.begin(), buttonsLast.end(), false);
	}

	void Mouse::Update()
	{
		xLast = x;
		yLast = y;
		buttonsLast = buttons;
		Uint32 state = SDL_GetMouseState(&x, &y);
		for (int i = 0; i < ButtonCount; i++)
		{
			buttons[i] = state & SDL_BUTTON(i + 1);
		}
	}

	bool Mouse::Button(int button)
	{
		BASED_ASSERT(button >= BASED_INPUT_MOUSE_FIRST && button <= BASED_INPUT_MOUSE_LAST, "Invalid mouse button!");
		if (button >= BASED_INPUT_MOUSE_FIRST && button <= BASED_INPUT_MOUSE_LAST)
		{
			return buttons[button - 1];
		}
		else
		{
			return false;
		}
	}

	bool Mouse::ButtonDown(int button)
	{
		BASED_ASSERT(button >= BASED_INPUT_MOUSE_FIRST && button <= BASED_INPUT_MOUSE_LAST, "Invalid mouse button!");
		if (button >= BASED_INPUT_MOUSE_FIRST && button <= BASED_INPUT_MOUSE_LAST)
		{
			return buttons[button - 1] && !buttonsLast[button - 1];
		}
		else
		{
			return false;
		}
	}

	bool Mouse::ButtonUp(int button)
	{
		BASED_ASSERT(button >= BASED_INPUT_MOUSE_FIRST && button <= BASED_INPUT_MOUSE_LAST, "Invalid mouse button!");
		if (button >= BASED_INPUT_MOUSE_FIRST && button <= BASED_INPUT_MOUSE_LAST)
		{
			return !buttons[button - 1] && buttonsLast[button - 1];
		}
		else
		{
			return false;
		}
	}
}
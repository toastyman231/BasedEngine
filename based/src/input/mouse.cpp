#include "input/mouse.h"
#include "log.h"

#include <algorithm>

#include "engine.h"
#include "input/keyboard.h"
#include "SDL2/SDL_mouse.h"
#include "based/core/profiler.h"

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
		PROFILE_FUNCTION();
		xLast = x;
		yLast = y;
		buttonsLast = buttons;
		Uint32 state = SDL_GetGlobalMouseState(&x, &y);
		for (int i = 0; i < ButtonCount; i++)
		{
			buttons[i] = state & SDL_BUTTON(i + 1);
		}

#ifdef BASED_CONFIG_DEBUG
		if (Keyboard::KeyDown(BASED_INPUT_KEY_ESCAPE))
		{
			SetCursorLocked(false);
		}
#endif
	}

	bool Mouse::Button(int button)
	{
		PROFILE_FUNCTION();
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
		PROFILE_FUNCTION();
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
		PROFILE_FUNCTION();
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

	void Mouse::SetCursorLocked(bool locked)
	{
		mCursorLocked = locked;
		SDL_SetRelativeMouseMode((locked) ? SDL_TRUE : SDL_FALSE);
	}

	glm::ivec2 Mouse::GetMousePosition()
	{
		int x;
		int y;
		Uint32 state = SDL_GetMouseState(&x, &y);
		return {x, y};
	}
}

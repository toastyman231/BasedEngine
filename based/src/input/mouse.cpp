#include "pch.h"
#include "input/mouse.h"

#include "engine.h"
#include "input/keyboard.h"
#include "SDL2/SDL_mouse.h"

namespace based::input
{
	int Mouse::x = 0;
	int Mouse::xLast = 0;
	int Mouse::y = 0;
	int Mouse::yLast = 0;
	CursorMode Mouse::mCursorMode = CursorMode::Confined;

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

#ifdef BASED_CONFIG_DEBUG
		if (mDebugTabbedOut)
		{
			int xTemp;
			int yTemp;
			if (SDL_GetMouseState(&xTemp, &yTemp) & SDL_BUTTON(1))
			{
				SetCursorMode(CursorMode::Confined);
				SetCursorVisible(false);
				mDebugTabbedOut = false;
			}

			return;
		}
#endif

		xLast = x;
		yLast = y;
		buttonsLast = buttons;
		Uint32 state = SDL_GetMouseState(&x, &y);
		glm::vec2 size = based::Engine::Instance().GetWindow().GetSize();
		if (mCursorMode == CursorMode::Confined && (x <= 1 || x >= static_cast<int>(size.x) - 1))
		{
			int newX = x <= 1 ? static_cast<int>(size.x) - 2 : 2;
			SDL_WarpMouseInWindow(based::Engine::Instance().GetWindow().GetSDLWindow(), newX, y);
			x = newX;
			xLast = newX;
		}
		if (mCursorMode == CursorMode::Confined && (y <= 1 || y >= static_cast<int>(size.y) - 1))
		{
			int newY = y <= 1 ? static_cast<int>(size.y) - 2 : 2;
			SDL_WarpMouseInWindow(based::Engine::Instance().GetWindow().GetSDLWindow(), x, newY);
			y = newY;
			yLast = newY;
		}

		for (int i = 0; i < ButtonCount; i++)
		{
			buttons[i] = state & SDL_BUTTON(i + 1);
		}

#ifdef BASED_CONFIG_DEBUG
		if (Keyboard::KeyDown(BASED_INPUT_KEY_ESCAPE))
		{
			SetCursorMode(CursorMode::Free);
			SetCursorVisible(true);
			mDebugTabbedOut = true;
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

	void Mouse::SetCursorMode(CursorMode mode)
	{
		mCursorMode = mode;
	}

	void Mouse::SetCursorVisible(bool visible)
	{
		SDL_ShowCursor(visible);
	}

	glm::ivec2 Mouse::GetMousePosition()
	{
		int x;
		int y;
		Uint32 state = SDL_GetMouseState(&x, &y);
		return {x, y};
	}
}

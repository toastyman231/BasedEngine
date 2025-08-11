#include "pch.h"
#include "input/mouse.h"

#include <SDL2/SDL_events.h>

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
		auto xMin = xBounds.x != -999.f ? static_cast<int>(xBounds.x) : 1;
		auto xMax = xBounds.y != -999.f ? static_cast<int>(xBounds.y) : static_cast<int>(size.x) - 1;
		if (mCursorMode == CursorMode::Confined && (x <= xMin || x >= xMax))
		{
			int newX = x <= xMin ? xMax - 1 : xMin + 1;
			SDL_WarpMouseInWindow(based::Engine::Instance().GetWindow().GetSDLWindow(), newX, y);
			x = newX;
			xLast = newX;
		}
		auto yMin = yBounds.x != -999.f ? static_cast<int>(yBounds.x) : 1;
		auto yMax = yBounds.y != -999.f ? static_cast<int>(yBounds.y) : static_cast<int>(size.y) - 1;
		if (mCursorMode == CursorMode::Confined && (y <= yMin || y >= yMax))
		{
			int newY = y <= yMin ? yMax - 1 : yMin + 1;
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

		// TODO: Figure this out
		/*switch (mode)
		{
		case CursorMode::Free:
			SDL_SetRelativeMouseMode(SDL_FALSE);
			break;
		case CursorMode::Confined:
			SDL_SetRelativeMouseMode(SDL_TRUE);
			break;
		}*/
	}

	void Mouse::SetCursorVisible(bool visible)
	{
		SDL_ShowCursor(visible);
	}

	void Mouse::SetMousePosition(glm::ivec2 position, bool zeroDelta)
	{
		SDL_bool wasPreviouslyRelative = SDL_GetRelativeMouseMode();
		int wasPreviouslyVisible = SDL_ShowCursor(SDL_QUERY);
		if (zeroDelta) SDL_SetRelativeMouseMode(SDL_TRUE);
		SDL_WarpMouseInWindow(Engine::Instance().GetWindow().GetSDLWindow(), position.x, position.y);
		if (zeroDelta)
		{
			x = position.x;
			y = position.y;
			xLast = x;
			yLast = y;
			SDL_SetRelativeMouseMode(wasPreviouslyRelative);
			SDL_ShowCursor(wasPreviouslyVisible);
		}
	}

	glm::ivec2 Mouse::GetMousePosition()
	{
		int x;
		int y;
		Uint32 state = SDL_GetMouseState(&x, &y);
		return {x, y};
	}
}

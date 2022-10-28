#include "input/keyboard.h"
#include "log.h"

#include <algorithm>

#include "SDL2/SDL_keyboard.h"

namespace based::input
{
	std::array<bool, Keyboard::KeyCount> Keyboard::keys;
	std::array<bool, Keyboard::KeyCount> Keyboard::keysLast;

	void Keyboard::Initialize()
	{
		std::fill(keys.begin(), keys.end(), false);
		std::fill(keysLast.begin(), keysLast.end(), false);
	}

	void Keyboard::Update()
	{
		keysLast = keys;
		const Uint8* state = SDL_GetKeyboardState(nullptr);
		for (int i = BASED_INPUT_KEY_FIRST; i < KeyCount; i++)
		{
			keys[i] = state[i];
		}
	}

	bool Keyboard::Key(int key)
	{
		BASED_ASSERT(key >= BASED_INPUT_KEY_FIRST && key < KeyCount, "Invalid keyboard key!");
		if (key >= BASED_INPUT_KEY_FIRST && key < KeyCount)
		{
			return keys[key];
		}
		else
		{
			return false;
		}
	}

	bool Keyboard::KeyDown(int key)
	{
		BASED_ASSERT(key >= BASED_INPUT_KEY_FIRST && key < KeyCount, "Invalid keyboard key!");
		if (key >= BASED_INPUT_KEY_FIRST && key < KeyCount)
		{
			return keys[key] && !keysLast[key];
		}
		else
		{
			return false;
		}
	}

	bool Keyboard::KeyUp(int key)
	{
		BASED_ASSERT(key >= BASED_INPUT_KEY_FIRST && key < KeyCount, "Invalid keyboard key!");
		if (key >= BASED_INPUT_KEY_FIRST && key < KeyCount)
		{
			return !keys[key] && keysLast[key];
		}
		else
		{
			return false;
		}
	}
}
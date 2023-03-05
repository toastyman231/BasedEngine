#pragma once

#include "SDL2/SDL.h"

namespace based::core
{
	class Time
	{
	private:
		inline static float mDeltaTime = 0.f;
	public:
		Time() = default;
		~Time() = default;

		static float GetTime()
		{
			return SDL_GetTicks64() / 1000.f;
		}

		static float DeltaTime()
		{
			return mDeltaTime;
		}

		static void SetDelta(float time)
		{
			mDeltaTime = time;
		}
	};
}

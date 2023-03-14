#pragma once

#include "SDL2/SDL.h"

namespace based::core
{
	class Time
	{
		friend class Engine;
	private:
		inline static float mLastFrameTime = 0.f;
		inline static float mDeltaTime = 0.f;
		inline static float mTimeScale = 1.f;
	public:
		Time() = default;
		~Time() = default;

		static float GetTime()
		{
			return SDL_GetTicks64() / 1000.f * mTimeScale;
		}

		static float DeltaTime()
		{
			return mDeltaTime;
		}

		static float TimeScale()
		{
			return mTimeScale;
		}

		static void SetDelta(float time, float delta)
		{
			mLastFrameTime = time;
			mDeltaTime = delta;
		}

		static void SetTimeScale(float timeScale)
		{
			mTimeScale = timeScale;
		}
	};
}

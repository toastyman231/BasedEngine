#pragma once
#include "Engine.h"

namespace based
{
    class Time final
    {
        friend class Engine;
    public:
        static float GetTime()
        {
            return mTime;
        }

        static float GetUnscaledTime()
        {
            return mUnscaledTime;
        }

        static float DeltaTime()
        {
            return mDeltaTime;
        }

        static float UnscaledDeltaTime()
        {
            return mUnscaledDeltaTime;
        }

        static float TimeScale()
        {
            return mTimeScale;
        }

        static void SetTimeScale(float timeScale)
        {
            if (timeScale < 0.f) return;
            mTimeScale = timeScale;
        }
        
    private:
        Time() = default;
        ~Time() = default;
            
        inline static float mLastFrameTime = 0.f;
        inline static float mUnscaledLastFrameTime = 0.f;
        inline static float mTime = 0.f;
        inline static float mDeltaTime = 0.f;
        inline static float mUnscaledDeltaTime = 0.f;
        inline static float mTimeScale = 1.f;
        inline static float mUnscaledTime = 0.f;
        inline static uint64_t mTicks = 0;

        static void SetDelta(float delta)
        {
            mLastFrameTime = mTime;
            mDeltaTime = delta;
        }

        static void SetUnscaledDelta(float dt)
        {
            mUnscaledLastFrameTime = mUnscaledTime;
            mUnscaledDeltaTime = dt;
        }

        static void UpdateUnscaledTime()
        {
            mUnscaledTime = PlatformEngine::GetEngineTicks() / 1000.f;
        }

        static void UpdateTime()
        {
            if (mTimeScale <= 0.f) return;
            mTime += mUnscaledDeltaTime * mTimeScale;
        }
    };
}

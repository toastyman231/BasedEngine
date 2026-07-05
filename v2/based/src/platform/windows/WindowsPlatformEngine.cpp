#include "pch.h"
#include "platform/windows/WindowsPlatformEngine.h"

#include "SDL3/SDL_timer.h"

namespace based
{
    uint64 WindowsPlatformEngine::GetEngineTicks()
    {
        return SDL_GetTicks();
    }

    const std::string& WindowsPlatformEngine::GetPlatformName()
    {
        // Short string optimization should prevent this from making an allocation
        static std::string name("Windows");
        return name;
    }
}

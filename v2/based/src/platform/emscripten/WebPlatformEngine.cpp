#include "pch.h"
#include "platform/emscripten/WebPlatformEngine.h"

#include "SDL3/SDL_timer.h"

namespace based
{
    uint64 WebPlatformEngine::GetEngineTicks()
    {
        return SDL_GetTicks();
    }

    const std::string& WebPlatformEngine::GetPlatformName()
    {
        // Short string optimization should prevent this from making an allocation
        static std::string name("Web");
        return name;
    }
}

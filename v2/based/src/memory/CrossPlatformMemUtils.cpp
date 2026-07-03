#include "pch.h"
#include "memory/PlatformMemUtils.h"

#include <SDL3/SDL_stdinc.h>

#include "core/NewDelete.h"

namespace based
{
    void SetupThirdPartyMemoryCallbacks()
    {
        SDL_SetMemoryFunctions(my_malloc, my_calloc, my_realloc, my_free);

        JPH::Allocate = my_malloc;
        JPH::Free = my_free;
        JPH::Reallocate = JoltRealloc;
        JPH::AlignedAllocate = MemoryManager::MemAlign;
        JPH::AlignedFree = MemoryManager::MemFree;
    }
}

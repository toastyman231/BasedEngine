#pragma once
#include "../core/BasedTypes.h"

namespace based
{
    class MemoryManager final : public NonMoveable
    {
    public:
        MemoryManager() = delete;
        ~MemoryManager() = delete;
        
        [[nodiscard]] static void* MemAlign(size_t size, size_t alignment);
        [[nodiscard]] static void* MemRealloc(void* ptr, size_t size);
        static void MemFree(void* ptr);
    };
}

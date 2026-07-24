#pragma once

#include <concepts>

#include "MemoryManager.h"
#include "MemoryPoolHeader.h"

#define bytes_to_kb(x) ((x##ULL) / 1000ULL)
#define bytes_to_mb(x) ((x##ULL) / 1000ULL / 1000ULL)
#define bytes_to_gb(x) ((x##ULL) / 1000ULL / 1000ULL / 1000ULL)

#define kb_to_bytes(x) ((x##ULL) * 1000ULL)
#define mb_to_bytes(x) ((x##ULL) * 1000ULL * 1000ULL)
#define gb_to_bytes(x) ((x##ULL) * 1000ULL * 1000ULL * 1000ULL)

#define bytes_to_kib(x) ((x##ULL) / 1024ULL)
#define bytes_to_mib(x) ((x##ULL) / 1024ULL / 1024ULL)
#define bytes_to_gib(x) ((x##ULL) / 1024ULL / 1024ULL / 1024ULL)

#define kib_to_bytes(x) ((x##ULL) * 1024ULL)
#define mib_to_bytes(x) ((x##ULL) * 1024ULL * 1024ULL)
#define gib_to_bytes(x) ((x##ULL) * 1024ULL * 1024ULL * 1024ULL)

namespace based
{
    void* AllocateSystemMemory(size_t bytes, bool bReserve = true, bool bCommit = true, bool bAllowAccess = true,
        void* pBaseAddress = nullptr);
    void* AllocateSystemMemory(size_t bytes, size_t alignment, bool bReserve = true, bool bCommit = true,
        bool bAllowAccess = true, void* pBaseAddress = nullptr);

    void SetupMemoryPools();
    // Define this in the graphics backend, since one platform can have multiple backends
    void SetupGraphicsPools();

    template <typename T> requires std::integral<T>
    constexpr T AlignUp(T value, T alignment) noexcept
    {
        return (value + (alignment - 1)) & ~(alignment - 1);
    }

    template <typename T>
    requires std::is_pointer_v<T>
    T AlignUpPtr(T ptr, uintptr_t alignment) noexcept
    {
        uintptr_t int_val = reinterpret_cast<uintptr_t>(ptr);
        uintptr_t aligned = (int_val + (alignment - 1)) & ~(alignment - 1);
        return reinterpret_cast<T>(aligned);
    }

    constexpr size_t GetPlatformDefaultAlignment() { return 2 * sizeof(void*); }

    size_t GetAvailableSystemMemoryBytes();
    size_t GetTotalSystemMemoryBytes();

    bool ValidateMemoryPoolSettings(const EngineMemoryPoolDescriptorList& poolList);
    bool ValidateGraphicsMemoryPoolSettings(const EngineMemoryPoolDescriptorList& poolList);

    // This is weak so that users can override this and provide their own descriptors of the
    // engine pools. Then when linking with the engine, their descriptors will be used instead of
    // the default. The returned list must be a static constexpr reference to avoid allocations.
    const EngineMemoryPoolDescriptorList& GetMemoryPoolDescriptors();

    inline void* JoltRealloc(void* ptr, size_t oldSize, size_t newSize)
    {
        return MemoryManager::MemRealloc(ptr, newSize);
    }

    void SetupThirdPartyMemoryCallbacks();
}

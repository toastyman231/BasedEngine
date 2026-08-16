#pragma once
#include "memory/MemoryManager.h"
#include "memory/MemoryPoolHeader.h"

// Standard Library extensions

namespace based
{
    // We don't specify a default pool so we're forced to make it explicit at callsites
    template <typename T, ePoolIdentifier Pool>
    class BasedPoolAllocator
    {
    public:
        using value_type = T;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;
        using pointer = T*;
        using const_pointer = const T*;

        BasedPoolAllocator() noexcept = default;

        template <typename U, ePoolIdentifier OtherPool>
        BasedPoolAllocator(const BasedPoolAllocator<U, OtherPool>& other) noexcept {}

        template <typename U>
        struct rebind {
            using other = BasedPoolAllocator<U, Pool>;
        };

        [[nodiscard]] T* allocate(size_t size)
        {
            AllocatorScope ac(Pool);

            return new T();
        }

        void deallocate(T* ptr, size_t size)
        {
            AllocatorScope ac(Pool);

            MemoryManager::MemFree(ptr);
        }
    };

    template <typename T, typename U, ePoolIdentifier Pool>
    bool operator==(const BasedPoolAllocator<T, Pool>&, const BasedPoolAllocator<U, Pool>&) noexcept
    {
        return true; // SFINAE checks the pool is the same for us, beyond that we don't really care
    }

    template <typename T, typename U, ePoolIdentifier Pool, ePoolIdentifier OtherPool>
    bool operator==(const BasedPoolAllocator<T, Pool>&, const BasedPoolAllocator<U, OtherPool>&) noexcept
    {
        return false; // Allocators with different pools are never equal
    }

    template <typename T>
    using PersistentPoolAllocator = BasedPoolAllocator<T, ePoolIdentifier::kPersistentPool>;
}

// Containers with an allocator set to use the persistent pool
namespace std_p
{
    template <typename T>
    using vector = std::vector<T, based::PersistentPoolAllocator<T>>;

    template <typename K, typename V>
    using map = std::map<K, V, based::PersistentPoolAllocator<std::pair<const K, V>>>;

    template <typename K, typename V>
    using unordered_map = std::unordered_map<K, V, based::PersistentPoolAllocator<std::pair<const K, V>>>;
}

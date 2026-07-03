#include "pch.h"
#include "memory/MemoryManager.h"

#include "memory/MemoryPoolAllocator.h"
#include "memory/MemoryPoolHeader.h"
#include "memory/PlatformMemUtils.h"

namespace based
{
    static BootstrapAllocator* g_pBootstrapAllocator = nullptr;
    
    void* MemoryManager::MemAlign(size_t size, size_t alignment)
    {
        if (BootstrapAllocator::ShouldUseBootstrap())
        {
            if (!g_pBootstrapAllocator)
            {
                size_t bootstrapPoolControlSize = sizeof(BootstrapAllocator);
                void* pBootstrapPool = AllocateSystemMemory(bootstrapPoolControlSize + sizeof(MemoryPoolHeader));
                g_pBootstrapAllocator = BootstrapAllocator::Create(pBootstrapPool);

                if (!g_pBootstrapAllocator) return nullptr;
            }

            void* ptr = g_pBootstrapAllocator->Allocate(size, alignment);
            if (!ptr)
            {
                BASED_SIMPLE_ASSERT(false, "Couldn't allocate %llu bytes from bootstrap pool!", size);
            }
            return ptr;
        }

        if (g_pCurrentMemoryPool && g_pCurrentMemoryPool->m_pPoolAllocator)
        {
            void* ptr = g_pCurrentMemoryPool->m_pPoolAllocator->Allocate(size, alignment);
#if BASED_CONFIG_DEBUG
            if (ptr)
            {
                g_pCurrentMemoryPool->m_pPoolAllocator->TrackUsageForPool(g_pCurrentMemoryPool, ptr);
            } else
            {
                MemoryPoolHeader::PrintPoolsLayout();
                g_pCurrentMemoryPool->PrintPoolInfo();
                BASED_ASSERT_FMT(false, "Couldn't allocate {} from pool {}!", MemSize{size},
                    to_underlying(g_pCurrentMemoryPool->GetPoolID()));
            }
#endif
            return ptr;
        }

        BASED_FATAL("Trying to allocate when there is no valid pool! This could cause an OS allocation!");
        return nullptr;
    }

    void* MemoryManager::MemRealloc(void* ptr, size_t size)
    {
        BASED_ASSERT(ptr, "Can't reallocate a null pointer!");
        if (!ptr) return nullptr;

        if (g_pCurrentMemoryPool && g_pCurrentMemoryPool->m_pPoolAllocator)
        {
#if BASED_CONFIG_DEBUG
            size_t stOldSize = g_pCurrentMemoryPool->m_pPoolAllocator->GetSizeForAllocation(ptr);
#endif
            void* ptr_out = g_pCurrentMemoryPool->m_pPoolAllocator->Reallocate(ptr, size);
#if BASED_CONFIG_DEBUG
            if (ptr_out)
            {
                g_pCurrentMemoryPool->m_pPoolAllocator->TrackUsageForPool(g_pCurrentMemoryPool, ptr_out, stOldSize);
            } else
            {
                MemoryPoolHeader::PrintPoolsLayout();
                g_pCurrentMemoryPool->PrintPoolInfo();
                BASED_ASSERT_FMT(false, "Couldn't reallocate {} from pool {}!", MemSize{size},
                    to_underlying(g_pCurrentMemoryPool->GetPoolID()));
            }
#endif
            return ptr_out;
        }

        BASED_FATAL("Trying to reallocate when there is no valid pool! This could cause an OS allocation!");
        return nullptr;
    }

    void MemoryManager::MemFree(void* ptr)
    {
        if (!ptr) return;

        if (g_pBootstrapAllocator->IsPointerFromAllocator(ptr))
            return; // bootstrap allocs are never individually freed

        // In many cases this is a simple pointer bounds check, very quick and easy way to tell if we have the
        // right pool. If we definitely don't, then we spend the slightly extra effort to search for the correct one.
        const bool bIsInCurrentPool = g_pCurrentMemoryPool->m_pPoolAllocator
            && g_pCurrentMemoryPool->m_pPoolAllocator->IsPointerFromAllocator(ptr);
        
        if (g_pCurrentMemoryPool && g_pCurrentMemoryPool->m_pPoolAllocator && bIsInCurrentPool)
        {
            return g_pCurrentMemoryPool->m_pPoolAllocator->Deallocate(ptr);
        }
        
        if (g_pCurrentMemoryPool && !bIsInCurrentPool)
        {
            MemoryPoolHeader* pActualPool = MemoryPoolHeader::GetPoolForPointer(ptr);
            BASED_ASSERT_FMT(false, "Trying to de-allocate from pool {} but ptr was from pool {}!",
                g_pCurrentMemoryPool->GetPoolName(), pActualPool ? pActualPool->GetPoolName() : "INVALID POOL");
            if (pActualPool && pActualPool->m_pPoolAllocator)
                return pActualPool->m_pPoolAllocator->Deallocate(ptr); // In case we want to continue past the assert
        }

        BASED_FATAL("Unable to de-allocate {}", ptr);
    }
}

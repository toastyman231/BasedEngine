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

            return g_pBootstrapAllocator->Allocate(size, alignment);
        }

        if (g_pCurrentMemoryPool && g_pCurrentMemoryPool->m_pPoolAllocator)
        {
            return g_pCurrentMemoryPool->m_pPoolAllocator->Allocate(size, alignment);
        }

        BASED_FATAL("Trying to allocate when there is no valid pool! This could cause an OS allocation!");
        return nullptr;
    }

    void MemoryManager::MemFree(void* pPtr)
    {
        if (!pPtr) return;

        if (g_pBootstrapAllocator->IsPointerFromAllocator(pPtr))
            return; // bootstrap allocs are never individually freed

        const bool bIsInCurrentPool = g_pCurrentMemoryPool->m_pPoolAllocator
            && g_pCurrentMemoryPool->m_pPoolAllocator->IsPointerFromAllocator(pPtr);
        
        if (g_pCurrentMemoryPool && g_pCurrentMemoryPool->m_pPoolAllocator && bIsInCurrentPool)
        {
            return g_pCurrentMemoryPool->m_pPoolAllocator->Deallocate(pPtr);
        }
        if (g_pCurrentMemoryPool && !bIsInCurrentPool)
        {
            BASED_ASSERT_FMT(false, "Trying to de-allocate from pool {} but ptr was from pool TBA!",
                g_pCurrentMemoryPool->GetPoolName());
        }

        BASED_FATAL("Unable to de-allocate {}", pPtr);
    }
}

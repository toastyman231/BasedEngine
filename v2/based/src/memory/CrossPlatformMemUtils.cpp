#include "pch.h"
#include "memory/PlatformMemUtils.h"

#include <SDL3/SDL_stdinc.h>

#include "core/NewDelete.h"
#include "SDL3/SDL_cpuinfo.h"

namespace based
{
    void SetupThirdPartyMemoryCallbacks()
    {
        JPH::Allocate = my_malloc;
        JPH::Free = my_free;
        JPH::Reallocate = JoltRealloc;
        JPH::AlignedAllocate = MemoryManager::MemAlign;
        JPH::AlignedFree = MemoryManager::MemFree;
    }

    size_t GetTotalSystemMemoryBytes()
    {
        int ramMiB = SDL_GetSystemRAM();
        return ramMiB * 1024ULL * 1024ULL;
    }

    bool ValidateMemoryPoolSettings(const EngineMemoryPoolDescriptorList& poolList)
    {
        size_t totalSize = 0;
        for (const auto& [ePoolID, poolDescriptor] : poolList.pools | std::views::all)
        {
            BASED_ASSERT_FMT(poolDescriptor.m_ePoolID == to_underlying(ePoolID),
                "Pool {} does not have a matching ID in the descriptor! (Has {})", 
                to_underlying(ePoolID), poolDescriptor.m_ePoolID);
            
            // Skip the root pool and GPU pools since we set those up separately
            if (poolDescriptor.m_ePoolID == to_underlying(ePoolIdentifier::kRootPool) || poolDescriptor.m_bIsGPUPool) 
                continue;
            
            BASED_ASSERT(poolDescriptor.m_ePoolID != to_underlying(ePoolIdentifier::kInvalid),
                "Pool descriptor has invalid ID, did you define all the engine pools?");
            if (poolDescriptor.m_ePoolID == to_underlying(ePoolIdentifier::kInvalid)) return false;
            
            size_t stPoolSize = poolDescriptor.m_stPoolSize;
            const std::string_view strPoolName = poolDescriptor.m_strPoolName;
            BASED_ASSERT_FMT(stPoolSize > 0, "Invalid size {} for pool {}!", stPoolSize, poolDescriptor.m_ePoolID);
            if (stPoolSize <= 0) return false;

            ePoolIdentifier eParentPoolID = static_cast<ePoolIdentifier>(poolDescriptor.m_eParentPoolID);
            if (eParentPoolID != ePoolIdentifier::kInvalid)
            {
                const auto pParentDescriptor = poolList.Find(eParentPoolID);
                BASED_ASSERT_FMT(pParentDescriptor, "No pool descriptor found for pool {}!", poolDescriptor.m_eParentPoolID);
                if (!pParentDescriptor) return false;
                
                BASED_ASSERT_FMT(stPoolSize < pParentDescriptor->m_stPoolSize,
                        "Child pool {} ({}) is bigger than parent {} ({})!", strPoolName, MemSize{stPoolSize},
                            pParentDescriptor->m_strPoolName, MemSize{pParentDescriptor->m_stPoolSize});
                if (stPoolSize >= pParentDescriptor->m_stPoolSize) return false;
            } else
            {
                totalSize += stPoolSize;
            }
        }

#ifndef SKIP_AVAILABLE_MEMORY_CHECK
        size_t totalRAM = GetTotalSystemMemoryBytes();
        size_t availRAM = GetAvailableSystemMemoryBytes();
        BASED_ASSERT_FMT(totalSize <= totalRAM && totalSize <= availRAM,
            "System does not have enough memory! Want: {}, Have: {}, Available: {}", totalSize, totalRAM, availRAM);
        if (totalSize > totalRAM || totalSize > availRAM) return false;
#else
        size_t totalRAM = GetTotalSystemMemoryBytes();
        BASED_ASSERT_FMT(totalSize <= totalRAM,
            "System does not have enough memory! Want: {}, Have: {}", totalSize, totalRAM);
        if (totalSize > totalRAM) return false;
#endif

        return true;
    }

    void SetupMemoryPools()
    {
        static bool bDoOnce = false;
        if (bDoOnce) return;

        MemoryPoolHeader::CreateRootPool();

        const EngineMemoryPoolDescriptorList& poolList = GetMemoryPoolDescriptors();
#ifdef BASED_CONFIG_DEBUG
        const bool bSuccess = ValidateMemoryPoolSettings(poolList);
        BASED_ASSERT(bSuccess, "Invalid pool settings!");
#endif
        
        for (const auto& poolDescriptor : poolList.pools | std::views::values)
        {
            // Skip the invalid and root pool identifiers, and any pools that haven't been defined (for now)
            if (poolDescriptor.m_ePoolID == to_underlying(ePoolIdentifier::kInvalid)
                || poolDescriptor.m_ePoolID == to_underlying(ePoolIdentifier::kRootPool)
                || poolDescriptor.m_stPoolSize == 0) continue;
            
            size_t stPoolSize = poolDescriptor.m_stPoolSize;
            ePoolIdentifier eParentPoolID = static_cast<ePoolIdentifier>(poolDescriptor.m_eParentPoolID);

            void* pBackingMem = nullptr;

            // Since VMA requires Vulkan to be set up, we're forced to setup GPU mem pools separately on that
            // platform. Therefore, all platforms set up GPU mempools separately. 
            if (eParentPoolID != ePoolIdentifier::kInvalid)
            {
                AllocatorScope ac(eParentPoolID);
                pBackingMem = new uint8[stPoolSize + sizeof(MemoryPoolHeader) + sizeof(MemPoolTLSFAllocator)];
            } else
            {
                pBackingMem = AllocateSystemMemory(
                    stPoolSize + sizeof(MemoryPoolHeader) + sizeof(MemPoolTLSFAllocator));
            }
            
            BASED_ASSERT(pBackingMem, "Invalid memory allocated!");
            MemoryPoolHeader::CreatePool(poolDescriptor.m_strPoolName,
                static_cast<ePoolIdentifier>(poolDescriptor.m_ePoolID),
                stPoolSize, pBackingMem);
        }

        BASED_INFO("Finished setting up memory pools!");
        
        bDoOnce = true;
    }
}

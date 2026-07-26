#include "pch.h"

#include "BasedDefines.h"
#include "memory/PlatformMemUtils.h"
#include "core/BasedLog.h"
#include "core/NewDelete.h"
#include "memory/MemoryPoolAllocator.h"
#include "memory/MemoryPoolHeader.h"

namespace based
{
    void* AllocateSystemMemory(size_t bytes, bool bReserve /*= true*/, bool bCommit /*= true*/,
        bool bAllowAccess /*= true*/, void* pBaseAddress /*= nullptr*/)
    {
        uint32 flags = 0;
        if (bReserve) flags |= MEM_RESERVE;
        if (bCommit) flags |= MEM_COMMIT;
        return VirtualAlloc(pBaseAddress, bytes, flags,
            bAllowAccess ? PAGE_READWRITE : PAGE_NOACCESS);
    }

    void* AllocateSystemMemory(size_t bytes, size_t alignment, bool bReserve /*= true*/, bool bCommit /*= true*/,
        bool bAllowAccess /*= true*/, void* pBaseAddress /*= nullptr*/)
    {
        uint32 flags = 0;
        if (bReserve) flags |= MEM_RESERVE;
        if (bCommit) flags |= MEM_COMMIT;
        return VirtualAlloc(pBaseAddress, AlignUp(bytes, alignment), flags,
            bAllowAccess ? PAGE_READWRITE : PAGE_NOACCESS);
    }

    size_t GetAvailableSystemMemoryBytes()
    {
        MEMORYSTATUSEX status;
        status.dwLength = sizeof(status);
        if (GlobalMemoryStatusEx(&status)) {
            return status.ullAvailPageFile; // Free physical RAM + pagefile
        }
        return 0;
    }

    BASED_WEAK(const EngineMemoryPoolDescriptorList&, GetMemoryPoolDescriptors)
    {
        /**
         * TODO: These are just random test values, users should set their own per-app anyway
         **/
        static constexpr EngineMemoryPoolDescriptorList DefaultPoolDescriptors = {
            .pools = {{
                { 
                    ePoolIdentifier::kPersistentPool, 
                    PoolDescriptor
                    {
                        .m_strPoolName = "Persistent",
                        .m_stPoolSize = gib_to_bytes(3),
                        .m_ePoolID = to_underlying(ePoolIdentifier::kPersistentPool),
                        .m_eParentPoolID = to_underlying(ePoolIdentifier::kInvalid),
                        .m_bIsGPUPool = false
                    }
                },
                { 
                    ePoolIdentifier::kScratchCPUPool, 
                    PoolDescriptor
                    {
                        .m_strPoolName = "Scratch",
                        .m_stPoolSize = gib_to_bytes(1),
                        .m_ePoolID = to_underlying(ePoolIdentifier::kScratchCPUPool),
                        .m_eParentPoolID = to_underlying(ePoolIdentifier::kPersistentPool),
                        .m_bIsGPUPool = false
                    }
                },
                { 
                    ePoolIdentifier::kStagingPool, 
                    PoolDescriptor
                    {
                        .m_strPoolName = "Staging",
                        .m_stPoolSize = gib_to_bytes(5),
                        .m_ePoolID = to_underlying(ePoolIdentifier::kStagingPool),
                        .m_eParentPoolID = to_underlying(ePoolIdentifier::kInvalid),
                        .m_bIsGPUPool = false
                    }
                },
            }}
        };

        return DefaultPoolDescriptors;
    }
}

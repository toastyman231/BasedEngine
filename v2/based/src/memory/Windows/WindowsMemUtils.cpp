#include "pch.h"

#include "core/BasedLog.h"
#include "memory/MemoryPoolAllocator.h"
#include "memory/MemoryPoolHeader.h"
#include "memory/PlatformMemUtils.h"

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

    void SetupMemoryPools()
    {
        static bool bDoOnce = false;
        if (bDoOnce) return;
        
        size_t persistentPoolSize = gib_to_bytes(3);
        void* pPersistentPool = AllocateSystemMemory(
             persistentPoolSize + sizeof(MemoryPoolHeader) + sizeof(MemPoolTLSFAllocator));
        BASED_ASSERT(pPersistentPool, "Invalid system memory allocated!");

        void* pCreatedPool = MemoryPoolHeader::CreatePool("Persistent", ePoolIdentifier::kPersistentPool,
            persistentPoolSize, pPersistentPool);
        BASED_ASSERT(pCreatedPool, "Pool creation failed for persistent pool!");
        
        bDoOnce = true;
    }
}

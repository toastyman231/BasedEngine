#include "pch.h"
#include "memory/MemoryPoolAllocator.h"

#include <tlsf.h>

#include "core/BasedLog.h"
#include "memory/PlatformMemUtils.h"

namespace based
{
    MemPoolTLSFAllocator* MemPoolTLSFAllocator::Create(void* pBackingMemory, size_t bytes, bool bZeroInit /*= true*/)
    {
        // Carve the allocator out of the backing memory
        uint8* pBasePtr = static_cast<uint8*>(pBackingMemory);
        MemPoolTLSFAllocator* pAllocator = static_cast<MemPoolTLSFAllocator*>(pBackingMemory);
        uint8* pUnalignedBase   = pBasePtr + sizeof(MemPoolTLSFAllocator);
        uint8* pAlignedBase     = AlignUpPtr(pUnalignedBase, alignof(std::max_align_t));
        size_t alignmentPadding = static_cast<size_t>(pAlignedBase - pUnalignedBase);
        size_t poolBytes        = bytes - alignmentPadding;

        if (bZeroInit) memset(pAlignedBase, 0, poolBytes);

        new (pAllocator) MemPoolTLSFAllocator();
        pAllocator->m_pBackingHeap = tlsf_create_with_pool(pAlignedBase, poolBytes);
        pAllocator->m_poolSize = poolBytes - tlsf_size(); // Don't count the tlsf control block
        BASED_ASSERT(pAlignedBase && pAllocator->m_pBackingHeap, "Something went wrong creating pool!");

        return pAllocator;
    }

    MemPoolTLSFAllocator::~MemPoolTLSFAllocator()
    {
        tlsf_destroy(m_pBackingHeap);
    }
    
    void* MemPoolTLSFAllocator::Allocate(size_t bytes)
    {
        return Allocate(bytes, GetPlatformDefaultAlignment());
    }
    
    void* MemPoolTLSFAllocator::Allocate(size_t size, size_t alignment)
    {
        // tlsf_malloc aligns to 8 bytes by default, typically 16 is more common, so I don't use that function
        return tlsf_memalign(m_pBackingHeap, alignment, size);
    }

    void MemPoolTLSFAllocator::Deallocate(void* ptr)
    {
        // The mem pool will check if the pointer belongs to this pool
        tlsf_free(m_pBackingHeap, ptr);
    }

    bool MemPoolTLSFAllocator::IsPointerFromAllocator(void* ptr) const
    {
        uint8* p = static_cast<uint8*>(ptr);
        uint8* pBaseAddress = static_cast<uint8*>(m_pBackingHeap) + tlsf_size(); // Skip the tlsf control block
        return p >= pBaseAddress && p < pBaseAddress + m_poolSize;
    }

    uint8 BootstrapAllocator::m_pBuffer[65536]; // 64kb of bootstrap memory should be fine
    size_t BootstrapAllocator::m_Offset = 0;
    bool BootstrapAllocator::m_bUseBootstrap = true;

    BootstrapAllocator* BootstrapAllocator::Create(void* pAllocatorMem)
    {
        static bool bCreatedBootstrapAllocator = false;
        BASED_SIMPLE_ASSERT(!bCreatedBootstrapAllocator, "Already created a bootstrap allocator! Use that one instead!");
        if (bCreatedBootstrapAllocator) return nullptr;
    
        BASED_SIMPLE_ASSERT(pAllocatorMem,
            "Invalid memory for the allocator control block! Pass a pointer to a block of at least %llu bytes!\n",
            sizeof(BootstrapAllocator));

        new (pAllocatorMem) BootstrapAllocator();
        bCreatedBootstrapAllocator = true;
        
        return static_cast<BootstrapAllocator*>(pAllocatorMem);
    }

    BootstrapAllocator::BootstrapAllocator()
    {
        m_Offset = 0;
        memset(m_pBuffer, 0, sizeof(m_pBuffer));
        m_bUseBootstrap = true;
    }

    void* BootstrapAllocator::Allocate(size_t bytes)
    {
        return Allocate(bytes, GetPlatformDefaultAlignment());
    }
    
    void* BootstrapAllocator::Allocate(size_t size, size_t alignment)
    {
        size_t alignedOffset = AlignUp(m_Offset, alignment);
        size_t end = alignedOffset + size;

        BASED_SIMPLE_ASSERT(end <= sizeof(m_pBuffer),
            "Bootstrap allocator exhausted trying to allocate %llu bytes!", size);

        void* ptr = m_pBuffer + alignedOffset;
        m_Offset = end;
        return ptr;
    }
    
    void BootstrapAllocator::Deallocate(void* ptr)
    {
        // We never free anything from the bootstrap pool
    }
    
    bool BootstrapAllocator::IsPointerFromAllocator(void* ptr) const
    {
        const void* pStart = m_pBuffer;
        const void* pEnd   = m_pBuffer + std::size(m_pBuffer);
        return ptr >= pStart && ptr < pEnd;
    }
}

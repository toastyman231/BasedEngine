#pragma once

#include "../core/BasedTypes.h"

namespace based
{
    class MemoryPoolHeader;

    // Intended for the TLSF allocator, but probably general enough to derive stats from any allocator
    struct PoolStats
    {
        size_t stTotalSize;         // Total pool size
        size_t stUsedBytes;         // Bytes in live allocations
        size_t stFreeBytes;         // Bytes in free blocks
        size_t stOverheadBytes;     // TLSF internal overhead
        size_t stPeakUsed;          // High watermark
    
        size_t stAllocationCount;   // Number of live allocations
        size_t stFreeBlockCount;    // Number of free blocks (fragmentation indicator)
    
        size_t stLargestFreeBlock;  // Largest single free block
        size_t stSmallestFreeBlock; // Smallest free block
        size_t stAverageFreeBlock;  // Average free block size

        float FragmentationRatio() const
        {
            if (stFreeBytes == 0) return 0.f;

            return 1.f - (static_cast<float>(stLargestFreeBlock) / 
                          static_cast<float>(stFreeBytes));
        }
    };
    
    class IMemoryPoolAllocator : public ExplicitlyCopyable
    {
    public:
        virtual ~IMemoryPoolAllocator() = default;
        
        [[nodiscard]] virtual void* Allocate(size_t bytes) = 0;
        [[nodiscard]] virtual void* Allocate(size_t size, size_t alignment) = 0;
        [[nodiscard]] virtual void* Reallocate(void* ptr, size_t size) = 0;

        virtual void Deallocate(void* ptr) = 0;
        [[nodiscard]] virtual bool IsPointerFromAllocator(void* ptr) const = 0;

        virtual PoolStats GetPoolStats(MemoryPoolHeader* pHeader) = 0;
        virtual void TrackUsageForPool(MemoryPoolHeader* pHeader, void* ptr, bool bIsFree = false,
            size_t stSizeBeforeRealloc = 0) {}
        virtual size_t GetSizeForAllocation(void* ptr) const { return 0; } // May not be implemented
    };

    class MemPoolTLSFAllocator final : public IMemoryPoolAllocator
    {
    public:
        
        static MemPoolTLSFAllocator* Create(void* pBackingMemory, size_t bytes, bool bZeroInit = true);
        
        [[nodiscard]] void* Allocate(size_t bytes) override;
        [[nodiscard]] void* Allocate(size_t size, size_t alignment) override;
                      void  Deallocate(void* ptr) override;
        [[nodiscard]] void* Reallocate(void* ptr, size_t size) override;
        [[nodiscard]] bool  IsPointerFromAllocator(void* ptr) const override;

        PoolStats GetPoolStats(MemoryPoolHeader* pHeader) override;
        void TrackUsageForPool(MemoryPoolHeader* pHeader, void* ptr, bool bIsFree = false,
            size_t stSizeBeforeRealloc = 0) override;
        size_t GetSizeForAllocation(void* ptr) const override;
        
    private:
        MemPoolTLSFAllocator() = default;
        ~MemPoolTLSFAllocator() override;

        void* m_pBackingHeap; // Actually tlsf_t
        size_t m_poolSize;
    };

    // Mostly just for setting up logging before the rest of the mem pools
    class BootstrapAllocator final : public IMemoryPoolAllocator
    {
        friend class MemoryManager;
    public:
        static BootstrapAllocator* Create(void* pAllocatorMem);
        
        [[nodiscard]] void* Allocate(size_t bytes) override;
        [[nodiscard]] void* Allocate(size_t size, size_t alignment) override;
                      void Deallocate(void* ptr) override;
        [[nodiscard]] void* Reallocate(void* ptr, size_t size) override;
        [[nodiscard]] bool IsPointerFromAllocator(void* ptr) const override;

        PoolStats GetPoolStats(MemoryPoolHeader* pHeader) override;

        static bool ShouldUseBootstrap() { return m_bUseBootstrap; }
        static void DisableBootstrap() { m_bUseBootstrap = false;}

    private:
        BootstrapAllocator();
        ~BootstrapAllocator() override = default;
        
        static uint8 m_pBuffer[65536]; // 64kb of bootstrap memory should be fine
        static size_t m_Offset;
        static bool m_bUseBootstrap;
    };
}

#pragma once

#include "../core/BasedTypes.h"

namespace based
{
    class IMemoryPoolAllocator : public ExplicitlyCopyable
    {
    public:
        virtual ~IMemoryPoolAllocator() = default;
        
        [[nodiscard]] virtual void* Allocate(size_t bytes) = 0;
        [[nodiscard]] virtual void* Allocate(size_t size, size_t alignment) = 0;

        virtual void Deallocate(void* ptr) = 0;
        [[nodiscard]] virtual bool IsPointerFromAllocator(void* ptr) const = 0;
    };

    class MemPoolTLSFAllocator final : public IMemoryPoolAllocator
    {
    public:
        
        static MemPoolTLSFAllocator* Create(void* pBackingMemory, size_t bytes, bool bZeroInit = true);
        
        [[nodiscard]] void* Allocate(size_t bytes) override;
        [[nodiscard]] void* Allocate(size_t size, size_t alignment) override;
                      void  Deallocate(void* ptr) override;
        [[nodiscard]] bool  IsPointerFromAllocator(void* ptr) const override;
        
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
        [[nodiscard]] bool IsPointerFromAllocator(void* ptr) const override;

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

#pragma once
#include "MemoryPoolAllocator.h"
#include "../core/BasedTypes.h"
#include "../core/BasedLog.h"

namespace based
{
    struct MemoryPoolListElement;
    
    constexpr size_t kMaxPools = 255;
    enum class ePoolIdentifier : uint8
    {
        kInvalid = 0,
        kRootPool,
        kPersistentPool,
        kPersistentGraphicsPool,
        kTextureCPUPool,
        kTextureGPUPool,
        kTextureSmallGPUPool,
        kScratchCPUPool,
        kStagingPool,
        // Any future engine pools go here
        kEnginePoolsCount = kStagingPool,
        kMaxEnginePools = 30,
        // Values after kMaxEnginePools are reserved for user pools
        kMaxPools = static_cast<uint8>(kMaxPools)
    };
    
    // We really want the backing memory to immediately follow this header (for CPU pools anyway)
    // so we make it NonMoveable (which includes NonCopyable) just to drive this home.
    // These pools aren't intended to be resizeable at runtime anyway
    class MemoryPoolHeader final : public NonMoveable
    {
        friend class MemoryManager;
    public:
        // Create a CPU pool using a pre-allocated chunk of memory
        template <typename T> requires EnumClassWithUnderlying<T, uint8>
        static MemoryPoolHeader* CreatePool(const char* pPoolName, T ePoolId, size_t stPool, void* pBackingMemory)
        {
            BASED_ASSERT(ms_bCreatedRootPool, "Must create root pool first!");
            BASED_ASSERT(pBackingMemory, "Invalid backing memory!");
            BASED_ASSERT(to_underlying(ePoolId) > 0, "Must pass a valid pool ID!");
            // TODO: Add proper enum printing w/ reflect-cpp
            BASED_ASSERT_FMT(ms_poolList[to_underlying(ePoolId)] == nullptr, "Already created a pool with ID {}!", to_underlying(ePoolId));
        
            uint8_t* pByteBase = static_cast<uint8_t*>(pBackingMemory);

            MemoryPoolHeader* pHeader = reinterpret_cast<MemoryPoolHeader*>(pByteBase);
            uint8_t* pAllocatorOffset = pByteBase + sizeof(MemoryPoolHeader);

            MemPoolTLSFAllocator* pAllocator = MemPoolTLSFAllocator::Create(pAllocatorOffset, stPool);
            uint8_t* pTrueBackingMemory = pAllocatorOffset + sizeof(MemPoolTLSFAllocator);
            BASED_ASSERT(pHeader, "Header allocation failed!");
            BASED_ASSERT(pAllocator, "Allocator registration failed!");

            new (pHeader) MemoryPoolHeader();
            strncpy_s(pHeader->m_pName, pPoolName, MAX_NAME_LEN - 1);
            pHeader->m_pName[MAX_NAME_LEN - 1] = '\0';
            pHeader->m_nPoolID = to_underlying(ePoolId);
            pHeader->m_stPoolSizeBytes = stPool;
            pHeader->m_pPoolAllocator = pAllocator;
            pHeader->m_pBackingMemory = pTrueBackingMemory;

            AddAndSplitPoolList(pHeader);

            return pHeader;
        }
        // Create a pool using an existing allocator and chunk of memory (usually a GPU pool)
        template <typename T> requires EnumClassWithUnderlying<T, uint8>
        static MemoryPoolHeader* CreatePool(const char* pPoolName, T ePoolId, size_t stPool,
            IMemoryPoolAllocator* pAllocator, void* pBackingMemory)
        {
            BASED_ASSERT(ms_bCreatedRootPool, "Must create root pool first!");
            BASED_ASSERT(pAllocator, "Invalid allocator pointer!");
            BASED_ASSERT(to_underlying(ePoolId) > 0, "Must pass a valid pool ID!");
            BASED_ASSERT_FMT(ms_poolList[to_underlying(ePoolId)] == nullptr, "Already created a pool with ID {}!", to_underlying(ePoolId));
        
            MemoryPoolHeader* pHeader = static_cast<MemoryPoolHeader*>(pBackingMemory);
            void* pTrueBackingMemory = pHeader + 1;
            BASED_ASSERT(pHeader, "Header pointer is not valid! Check where you got the backing memory from!");
            BASED_ASSERT(pTrueBackingMemory, "Memory after the header is invalid! Did your allocation get corrupted?");
        
            new (pHeader) MemoryPoolHeader();
            strncpy_s(pHeader->m_pName, pPoolName, MAX_NAME_LEN - 1);
            pHeader->m_pName[MAX_NAME_LEN - 1] = '\0';
            pHeader->m_nPoolID = to_underlying(ePoolId);
            pHeader->m_stPoolSizeBytes = stPool;
            pHeader->m_pPoolAllocator = pAllocator;
            pHeader->m_pBackingMemory = pTrueBackingMemory;

            AddAndSplitPoolList(pHeader);

            return pHeader;
        }

        // We don't actually want people to use this pool, it's just for tracking stuff, so we hide it from the user
        static void CreateRootPool();
        static void AddAndSplitPoolList(MemoryPoolHeader* pHeader);
        static size_t GetPoolIndexForPointer(void* ptr);
        static MemoryPoolHeader* GetPoolForPointer(void* ptr);
        static bool DoesPoolContainPointer(void* ptr, const MemoryPoolHeader* pPool);
        static void PrintPoolsLayout();
        
        template <typename T> requires EnumClassWithUnderlying<T, uint8>
        static MemoryPoolHeader* GetPoolByID(T nID)
        {
            BASED_ASSERT(to_underlying(nID) > static_cast<uint8>(ePoolIdentifier::kInvalid), "Invalid pool ID!");
            BASED_ASSERT(to_underlying(nID) != static_cast<uint8>(ePoolIdentifier::kRootPool), "You cannot access the root pool!");
            return ms_poolList[to_underlying(nID)];
        }

        [[nodiscard]] size_t GetPoolSize() const { return m_stPoolSizeBytes; }
        [[nodiscard]] std::string_view GetPoolName() const { return {m_pName}; }
        [[nodiscard]] ePoolIdentifier GetPoolID() const { return static_cast<ePoolIdentifier>(m_nPoolID); }

        // Users declaring custom pools should prefer this version of GetPoolID, since it will nicely return their
        // custom pool ID type
        template <typename T> requires EnumClassWithUnderlying<T, uint8>
        [[nodiscard]] T GetUserPoolID() const
        {
            BASED_ASSERT(m_nPoolID > to_underlying(ePoolIdentifier::kMaxEnginePools),
                "User pool ID out of valid range, use GetPoolID() instead!");
            return static_cast<T>(m_nPoolID);
        }
        
    private:
        MemoryPoolHeader() = default;
        ~MemoryPoolHeader() = default;

        static bool ms_bCreatedRootPool;
        static MemoryPoolHeader* ms_poolList[kMaxPools];
        static MemoryPoolListElement ms_poolListSorted[kMaxPools]; // This will only be a problem if we cut our pools up like a lot

        static constexpr size_t MAX_NAME_LEN = 64;
        char   m_pName[MAX_NAME_LEN];
        uint8  m_nPoolID;
        size_t m_stPoolSizeBytes;
        IMemoryPoolAllocator* m_pPoolAllocator;
        void* m_pBackingMemory;
        /**
        * For a GPU pool, this backing memory is actually CPU memory, even if the pool is CPU-visible. 
        * This is because we can't store allocation info in GPU info, so we mirror the pool on the CPU and store
        * the allocations there. This also has the benefit of allowing us to check which pool GPU allocations belong
        * to and if they're valid when deallocating. 
        **/
    };

    // We're never going to be creating these ourselves, just using the ones already in the static list
    struct MemoryPoolListElement final 
    {
        MemoryPoolHeader* m_pPool;
        MemoryPoolHeader* m_pParentPool;
        uint8* m_pStartAddress;
        uint8* m_pEndAddress;
        size_t m_poolSize;
    };

    class AllocatorScope final : public NonMoveable
    {
    public:
        AllocatorScope() = delete;
        AllocatorScope(MemoryPoolHeader* pMemoryPool);
        AllocatorScope(MemoryPoolHeader* pMemoryPool, MemoryPoolHeader* pGraphicsPool);
        AllocatorScope(ePoolIdentifier poolID);
        AllocatorScope(ePoolIdentifier poolID, ePoolIdentifier graphicsPoolID);
        ~AllocatorScope();

    private:
        MemoryPoolHeader* m_pPreviousPool, *m_pPreviousGraphicsPool;
    };

    // Prefer not setting these directly, use AllocatorScope instead
    // Because they're thread_local, returning them from a function could introduce some overhead,
    // so we just allow everyone to reference them directly where needed
    inline thread_local MemoryPoolHeader* g_pCurrentMemoryPool = nullptr;
    inline thread_local MemoryPoolHeader* g_pCurrentGraphicsPool = nullptr;

}

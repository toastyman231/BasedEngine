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
        BASED_ASSERT(pBaseAddress == nullptr,
        "AllocateSystemMemory: fixed base address not supported under Emscripten");
        BASED_ASSERT(bAllowAccess,
            "AllocateSystemMemory: PAGE_NOACCESS has no equivalent under Emscripten");

        if (!bCommit)
            return nullptr;

        size_t stCurrentHeapEnd = emscripten_get_heap_size();
        size_t stRequired = stCurrentHeapEnd + bytes;

        if (!emscripten_resize_heap(stRequired))
        {
            BASED_ASSERT(false, "Failed to grow WASM heap");
            return nullptr;
        }

        return reinterpret_cast<void*>(stCurrentHeapEnd);
    }

    void* AllocateSystemMemory(size_t bytes, size_t alignment, bool bReserve /*= true*/, bool bCommit /*= true*/,
        bool bAllowAccess /*= true*/, void* pBaseAddress /*= nullptr*/)
    {
        BASED_ASSERT(pBaseAddress == nullptr,
        "AllocateSystemMemory: fixed base address not supported under Emscripten");
        BASED_ASSERT(bAllowAccess,
            "AllocateSystemMemory: PAGE_NOACCESS has no equivalent under Emscripten");

        if (!bCommit)
            return nullptr;

        size_t stCurrentHeapEnd = emscripten_get_heap_size();
        size_t stRequired = stCurrentHeapEnd + AlignUp(bytes, alignment);

        if (!emscripten_resize_heap(stRequired))
        {
            BASED_ASSERT(false, "Failed to grow WASM heap");
            return nullptr;
        }

        return reinterpret_cast<void*>(stCurrentHeapEnd);
    }

    size_t GetAvailableSystemMemoryBytes()
    {
        size_t stUsed = emscripten_get_heap_size();
        size_t stMax = emscripten_get_heap_max();
        BASED_TRACE("Heap size: {}, Heap max: {}", MemSize{stUsed}, MemSize{stMax});
        return stMax - stUsed;
    }

    BASED_WEAK const EngineMemoryPoolDescriptorList& GetMemoryPoolDescriptors()
    {
        /**
         * TODO: These are just random test values, the user should really be overriding these for their project
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

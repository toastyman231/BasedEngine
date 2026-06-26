#include <algorithm>

#include "pch.h"
#include "memory/MemoryPoolHeader.h"

#include "core/BasedLog.h"

namespace based
{
    MemoryPoolHeader* MemoryPoolHeader::ms_poolList[kMaxPools] = { nullptr };
    MemoryPoolListElement MemoryPoolHeader::ms_poolListSorted[kMaxPools] = {};
    bool MemoryPoolHeader::ms_bCreatedRootPool = false;
    static uint8 s_rootPoolHeaderMem[sizeof(MemoryPoolHeader)]; // Memory for the root pool header
    static constexpr size_t s_nInvalidPoolIndex = 69420; // Just something higher than the max valid index
    static size_t s_nMaxPoolListIndex = s_nInvalidPoolIndex;
    static MemoryPoolHeader* s_pRootPool = nullptr;

    void MemoryPoolHeader::CreateRootPool()
    {
        static bool bDoOnce = false;
        BASED_ASSERT(!bDoOnce, "Root pool has already been created! Don't call this again.");
        if (bDoOnce) return;
        
        MemoryPoolHeader* pHeader = reinterpret_cast<MemoryPoolHeader*>(&s_rootPoolHeaderMem[0]);
        
        new (pHeader) MemoryPoolHeader();
        strncpy_s(pHeader->m_pName, "Root", MAX_NAME_LEN - 1);
        pHeader->m_pName[MAX_NAME_LEN - 1] = '\0';
        pHeader->m_nPoolID = to_underlying(ePoolIdentifier::kRootPool);
        pHeader->m_stPoolSizeBytes = 0xFFFFFFFFFFFFFFFF;
        pHeader->m_pPoolAllocator = nullptr;
        pHeader->m_pBackingMemory = nullptr;
        ms_poolListSorted[0] = {
            .m_pPool = pHeader,
            .m_pParentPool = nullptr,
            .m_pStartAddress = static_cast<uint8*>(pHeader->m_pBackingMemory),
            .m_pEndAddress = static_cast<uint8*>(pHeader->m_pBackingMemory) + pHeader->m_stPoolSizeBytes,
            .m_poolSize = pHeader->m_stPoolSizeBytes
        };
        
        bDoOnce = true;
        ms_bCreatedRootPool = true;
        s_nMaxPoolListIndex = 1;
        s_pRootPool = pHeader;
    }

    void MemoryPoolHeader::AddAndSplitPoolList(MemoryPoolHeader* pHeader)
    {
        BASED_ASSERT(pHeader, "Trying to add an invalid pool!");
        BASED_ASSERT(s_nMaxPoolListIndex != s_nInvalidPoolIndex, "Max pool index is invalid, did you init the root pool?");
        BASED_ASSERT(s_pRootPool, "Root pool is invalid, make sure to create it!");
        BASED_ASSERT(GetPoolByID(pHeader->GetPoolID()) == nullptr, "Trying to add a pool that's already been set up!");

        size_t index = GetPoolIndexForPointer(pHeader->m_pBackingMemory);
        BASED_ASSERT(index != s_nInvalidPoolIndex, "An invalid pool index was returned!");
        if (index == s_nInvalidPoolIndex) return;

        const MemoryPoolListElement& parentCopy = ms_poolListSorted[index];
        MemoryPoolHeader* pParent = parentCopy.m_pPool;
        MemoryPoolHeader* pParentsParent = parentCopy.m_pParentPool;
        uint8* pParentStart = parentCopy.m_pStartAddress;
        uint8* pParentEnd = parentCopy.m_pEndAddress;
        size_t pParentSize = parentCopy.m_poolSize;

        uint8* pPoolStart = static_cast<uint8*>(pHeader->m_pBackingMemory);
        uint8* pPoolEnd = pPoolStart + pHeader->m_stPoolSizeBytes;
        
        BASED_ASSERT_FMT(pPoolStart >= pParentStart && pPoolEnd <= pParentEnd,
            "Child pool {} does not lie entirely within parent pool {}!", pHeader->GetPoolName(), pParent->GetPoolName());
        BASED_ASSERT_FMT(!(pPoolStart == pParentStart && pPoolEnd == pParentEnd),
            "Child pool {} perfectly shadows parent pool {}! Increase parent size or decrease child size!",
            pHeader->GetPoolName(), pParent->GetPoolName());
        if (!(pPoolStart >= pParentStart && pPoolEnd <= pParentEnd)
            || (pPoolStart == pParentStart && pPoolEnd == pParentEnd)) return;

        size_t indexToInsertAt = index + 1;
        size_t indexToCopyFrom;
        size_t offset = 1;

        if (pPoolStart == pParentStart)
        {
            indexToInsertAt = index;
            indexToCopyFrom = indexToInsertAt; // Copy the old index forward
        } 
        else if (pPoolEnd == pParentEnd)
        {
            indexToCopyFrom = indexToInsertAt;
        } 
        else
        {
            indexToCopyFrom = indexToInsertAt;
            offset = 2;
        }
        
        BASED_ASSERT(s_nMaxPoolListIndex + offset <= kMaxPools, "Pool list exceeded maximum capacity!");
        
        if (indexToCopyFrom < s_nMaxPoolListIndex)
        {
            size_t numToCopy = s_nMaxPoolListIndex - indexToCopyFrom;
            MemoryPoolListElement* pCopySource = &ms_poolListSorted[indexToCopyFrom];
            MemoryPoolListElement* pCopyTo = pCopySource + offset;

            std::memmove(pCopyTo, pCopySource, sizeof(MemoryPoolListElement) * numToCopy);
        }

        if (pPoolStart == pParentStart)
        {
            ms_poolListSorted[indexToInsertAt + 1] = {
                .m_pPool = pParent,
                .m_pParentPool = pParentsParent, 
                .m_pStartAddress = pPoolEnd,
                .m_pEndAddress = pParentEnd,
                .m_poolSize = pParentSize - pHeader->m_stPoolSizeBytes
            };
        } 
        else if (pPoolEnd == pParentEnd)
        {
            ms_poolListSorted[index] = {
                .m_pPool = pParent,
                .m_pParentPool = pParentsParent,
                .m_pStartAddress = pParentStart,
                .m_pEndAddress = pPoolStart,
                .m_poolSize = pParentSize - pHeader->m_stPoolSizeBytes
            };
        } 
        else
        {
            // Left side remainder
            ms_poolListSorted[index] = {
                .m_pPool = pParent,
                .m_pParentPool = pParentsParent,
                .m_pStartAddress = pParentStart,
                .m_pEndAddress = pPoolStart,
                .m_poolSize = static_cast<size_t>(pPoolStart - pParentStart)
            };

            // Right side remainder
            ms_poolListSorted[indexToInsertAt + 1] = {
                .m_pPool = pParent,
                .m_pParentPool = pParentsParent,
                .m_pStartAddress = pPoolEnd,
                .m_pEndAddress = pParentEnd,
                .m_poolSize = static_cast<size_t>(pParentEnd - pPoolEnd)
            };
        }

        ms_poolListSorted[indexToInsertAt] = {
            .m_pPool = pHeader,
            .m_pParentPool = pParent,
            .m_pStartAddress = pPoolStart,
            .m_pEndAddress = pPoolEnd,
            .m_poolSize = pHeader->m_stPoolSizeBytes
        };

        s_nMaxPoolListIndex += offset;
        ms_poolList[to_underlying(pHeader->GetPoolID())] = pHeader;
    }

    size_t MemoryPoolHeader::GetPoolIndexForPointer(void* ptr)
    {
        BASED_ASSERT(s_nMaxPoolListIndex != s_nInvalidPoolIndex, "Max pool index is invalid, did you init the root pool?");
        uintptr_t targetAddr = reinterpret_cast<uintptr_t>(ptr);

        if (s_nMaxPoolListIndex == 0) return s_nInvalidPoolIndex;

        size_t min = 0;
        size_t max = s_nMaxPoolListIndex - 1;

        while (min <= max)
        {
            size_t mid = min + (max - min) / 2; 
            const MemoryPoolListElement& listElement = ms_poolListSorted[mid];

            uintptr_t startAddr = reinterpret_cast<uintptr_t>(listElement.m_pStartAddress);
            uintptr_t endAddr   = reinterpret_cast<uintptr_t>(listElement.m_pEndAddress);

            if (targetAddr < startAddr)
            {
                if (mid == 0) break; 
                max = mid - 1;
            }
            else if (targetAddr >= endAddr)
            {
                min = mid + 1;
            }
            else
            {
                return mid;
            }
        }

        return s_nInvalidPoolIndex;
    }

    MemoryPoolHeader* MemoryPoolHeader::GetPoolForPointer(void* ptr)
    {
        size_t index = GetPoolIndexForPointer(ptr);
        BASED_ASSERT(index <= s_nMaxPoolListIndex, "You somehow got a pointer outside what's been allocated!");
        if (index <= s_nMaxPoolListIndex) return nullptr;

        const MemoryPoolListElement& listElement = ms_poolListSorted[index];
        return listElement.m_pPool;
    }

    bool MemoryPoolHeader::DoesPoolContainPointer(void* ptr, const MemoryPoolHeader* pPool)
    {
        BASED_ASSERT(pPool, "Didn't pass a valid pool!");
        if (!pPool) return false;
        
        size_t index = GetPoolIndexForPointer(ptr);
        BASED_ASSERT(index <= s_nMaxPoolListIndex, "You somehow got a pointer outside what's been allocated!");
        if (index <= s_nMaxPoolListIndex) return false;

        const MemoryPoolListElement& listElement = ms_poolListSorted[index];
        MemoryPoolHeader* pHeader = listElement.m_pPool;
        if (pHeader == s_pRootPool) return false;

        return pHeader == pPool;
    }

    void MemoryPoolHeader::PrintPoolsLayout()
    {
        BASED_INFO("=================== MEMORY POOL LAYOUT DESCRIPTOR ===================");
    
        std::stringstream headerStream;
        headerStream << std::left 
                     << std::setw(20) << "Pool Name"
                     << std::setw(20) << "Start Addr"
                     << std::setw(20) << "End Addr"
                     << std::setw(21) << "Total Size (Bytes)"
                     << "Visual Span (Zoomed Linear)";
        BASED_INFO(headerStream.str().c_str());
        BASED_INFO(std::string(110, '-').c_str());

        // 1. Collect and sort all valid unique pools by memory address
        std::vector<MemoryPoolHeader*> sortedPools;
        for (size_t i = 0; i <= to_underlying(ePoolIdentifier::kEnginePoolsCount); ++i)
        {
            if (ms_poolList[i] != nullptr)
            {
                sortedPools.push_back(ms_poolList[i]);
            }
        }

        std::ranges::sort(sortedPools, [](MemoryPoolHeader* a, MemoryPoolHeader* b) {
            return reinterpret_cast<uintptr_t>(a->m_pBackingMemory) < reinterpret_cast<uintptr_t>(b->m_pBackingMemory);
        });
        sortedPools.insert(sortedPools.begin(), s_pRootPool);

        // 2. Scan for the absolute minimum and maximum boundaries of active sub-allocations
        uintptr_t globalMin = 0xFFFFFFFFFFFFFFFFULL;
        uintptr_t globalMax = 0;

        for (MemoryPoolHeader* pPool : sortedPools)
        {
            uintptr_t checkStart = reinterpret_cast<uintptr_t>(pPool->m_pBackingMemory);
            uintptr_t checkEnd = (pPool->m_stPoolSizeBytes == 0xFFFFFFFFFFFFFFFFULL)
                                ? 0
                                : (checkStart + pPool->m_stPoolSizeBytes);
            
            // Filter out extreme root bounds to prevent our zoom scale from breaking
            if (checkStart < globalMin && checkStart > 0) globalMin = checkStart;
            if (checkEnd > globalMax && checkEnd < 0xFFFFFFFFFFFFFFFFULL) globalMax = checkEnd;
        }

        // Fallback: If only the Root pool exists, capture its natural limits safely
        if (globalMin == 0xFFFFFFFFFFFFFFFFULL) globalMin = 0;
        if (globalMax == 0) globalMax = 0xFFFFFFFFFFFFFFFFULL;

        uintptr_t dynamicRange = globalMax - globalMin;

        // 3. Print each sorted unique pool line item
        for (MemoryPoolHeader* pPool : sortedPools)
        {
            uintptr_t poolStart = reinterpret_cast<uintptr_t>(pPool->m_pBackingMemory);
            uintptr_t poolEnd = (pPool->m_stPoolSizeBytes == 0xFFFFFFFFFFFFFFFFULL)
                                ? 0xFFFFFFFFFFFFFFFFULL
                                : (poolStart + pPool->m_stPoolSizeBytes);

            // 4. Compute nesting depth based on memory enclosures
            size_t depth = 0;
            for (MemoryPoolHeader* pTargetParent : sortedPools)
            {
                if (pTargetParent == pPool) continue;

                uintptr_t parentStart = reinterpret_cast<uintptr_t>(pTargetParent->m_pBackingMemory);
                uintptr_t parentEnd = (pTargetParent->m_stPoolSizeBytes == 0xFFFFFFFFFFFFFFFFULL)
                                      ? 0xFFFFFFFFFFFFFFFFULL
                                      : (parentStart + pTargetParent->m_stPoolSizeBytes);

                if (poolStart >= parentStart && poolEnd <= parentEnd)
                {
                    if (!(poolStart == parentStart && poolEnd == parentEnd))
                    {
                        depth++;
                    }
                }
            }

            std::string indentStr = (depth > 0) ? std::string(depth * 2, ' ') + "|- " : "";
            std::string displayName = indentStr.append(pPool->GetPoolName());
            if (displayName.length() > 19) displayName = displayName.substr(0, 16) + "...";

            // 5. Build row strings with clean column separation
            std::stringstream rowStream;
            rowStream << std::left << std::setw(20) << displayName;
            rowStream << "0x" << std::hex << std::setfill('0') << std::setw(16) << poolStart << "  ";
            rowStream << "0x" << std::hex << std::setfill('0') << std::setw(16) << poolEnd << "  ";
            rowStream << std::dec << std::setfill(' ') << std::setw(20) << pPool->m_stPoolSizeBytes << " ";

            // 6. Generate the Zoomed Linear Span Bar
            if (dynamicRange > 0)
            {
                // Clamp pointers to the active window to insulate against overflow/underflow
                uintptr_t clampStart = poolStart;
                uintptr_t clampEnd = poolEnd;

                clampStart = max(clampStart, globalMin);
                clampStart = min(clampStart, globalMax);

                clampEnd = max(clampEnd, globalMin);
                clampEnd = min(clampEnd, globalMax);

                const size_t kBarWidth = 30;
                double scale = static_cast<double>(kBarWidth) / static_cast<double>(dynamicRange);
                
                size_t startPos = static_cast<size_t>(static_cast<double>(clampStart - globalMin) * scale);
                size_t endPos = static_cast<size_t>(static_cast<double>(clampEnd - globalMin) * scale);

                startPos = min(startPos, kBarWidth);
                endPos   = min(endPos, kBarWidth);
                endPos   = max(endPos, startPos);

                // Maintain 1-cell visibility for pools that legitimately intersect our window bounds
                if (endPos == startPos && poolEnd > poolStart)
                {
                    if (endPos < kBarWidth) endPos++;
                    else if (startPos > 0) startPos--;
                }

                std::string visualBar(kBarWidth, '.');
                for (size_t b = startPos; b < endPos; ++b)
                {
                    visualBar[b] = '#'; 
                }
                rowStream << "[" << visualBar << "]";
            }
            else
            {
                rowStream << "[" << std::string(30, '#') << "]";
            }

            BASED_INFO(rowStream.str().c_str());
        }
        
        BASED_INFO(std::string(110, '=').c_str());
    }

    AllocatorScope::AllocatorScope(MemoryPoolHeader* pMemoryPool)
        : m_pPreviousPool(g_pCurrentMemoryPool)
        , m_pPreviousGraphicsPool(nullptr)
    {
        BASED_ASSERT(pMemoryPool, "Invalid memory pool passed to AllocatorScope!");
        g_pCurrentMemoryPool = pMemoryPool;
    }
    
    AllocatorScope::AllocatorScope(MemoryPoolHeader* pMemoryPool, MemoryPoolHeader* pGraphicsPool)
        : m_pPreviousPool(g_pCurrentMemoryPool)
        , m_pPreviousGraphicsPool(g_pCurrentGraphicsPool)
    {
        BASED_ASSERT(pMemoryPool && pGraphicsPool, "Invalid memory pools passed to AllocatorScope!");
        g_pCurrentMemoryPool = pMemoryPool;
        g_pCurrentGraphicsPool = pGraphicsPool;
    }
    
    AllocatorScope::AllocatorScope(ePoolIdentifier poolID)
    {
        m_pPreviousPool = g_pCurrentMemoryPool;
        m_pPreviousGraphicsPool = nullptr;

        g_pCurrentMemoryPool = MemoryPoolHeader::GetPoolByID(poolID);
        BASED_ASSERT_FMT(g_pCurrentMemoryPool, "Invalid memory pool {} passed to AllocatorScope!",
            static_cast<uint8>(poolID));
    }
    
    AllocatorScope::AllocatorScope(ePoolIdentifier poolID, ePoolIdentifier graphicsPoolID)
    {
        m_pPreviousPool = g_pCurrentMemoryPool;
        m_pPreviousGraphicsPool = g_pCurrentGraphicsPool;

        g_pCurrentMemoryPool = MemoryPoolHeader::GetPoolByID(poolID);
        g_pCurrentGraphicsPool = MemoryPoolHeader::GetPoolByID(graphicsPoolID);
        BASED_ASSERT_FMT(g_pCurrentMemoryPool && g_pCurrentGraphicsPool,
            "Invalid memory pools {} and {} passed to AllocatorScope!", static_cast<uint8>(poolID),
                static_cast<uint8>(graphicsPoolID));
    }
    
    AllocatorScope::~AllocatorScope()
    {
        if (m_pPreviousPool)
        {
            g_pCurrentMemoryPool = m_pPreviousPool;
        }

        if (m_pPreviousGraphicsPool)
        {
            g_pCurrentGraphicsPool = m_pPreviousGraphicsPool;
        }
    }
}

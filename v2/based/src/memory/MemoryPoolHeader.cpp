#include "pch.h"
#include "memory/MemoryPoolHeader.h"

#include "core/BasedLog.h"

namespace based
{
    MemoryPoolHeader* MemoryPoolHeader::s_PoolList[kMaxPools] = { nullptr };
    
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

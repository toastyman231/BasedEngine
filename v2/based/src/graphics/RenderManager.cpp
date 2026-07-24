#include "pch.h"
#include "graphics/GraphicsEngine.h"
#include "graphics/RenderManager.h"
#include "memory/MemoryPoolHeader.h"

namespace based
{
    RenderManager* RenderManager::Create()
    {
        static bool bDoOnce = false;
        BASED_ASSERT(!bDoOnce, "Trying to create a second RenderManager is not allowed! Get it from the Engine!");

        AllocatorScope ac(ePoolIdentifier::kPersistentPool);
        RenderManager* pRenderManager = new RenderManager();
        BASED_ASSERT(pRenderManager, "Something went wrong trying to create RenderManager!");
        if (pRenderManager)
        {
            bDoOnce = true;
            return pRenderManager;
        }

        return nullptr;
    }
    
    void RenderManager::Initialize()
    {
        AllocatorScope ac(ePoolIdentifier::kPersistentPool);
        m_pGraphicsEngine = dynamic_cast<IGraphicsEngine*>(new PlatformGraphicsEngine());
        BASED_ASSERT(m_pGraphicsEngine, "Error creating platform graphics engine!");
        if (m_pGraphicsEngine)
        {
            m_pGraphicsEngine->Initialize();
        }
    }
    
    void RenderManager::Shutdown()
    {
        AllocatorScope ac(ePoolIdentifier::kPersistentPool);
        BASED_ASSERT(m_pGraphicsEngine, "Invalid graphics engine! Has it been destroyed already?");
        if (m_pGraphicsEngine)
        {
            m_pGraphicsEngine->Shutdown();
        }
    }
}

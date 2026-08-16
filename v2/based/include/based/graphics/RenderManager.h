#pragma once

namespace based
{
    enum class eTextureFormat : uint8;
    
    enum class eAntiAliasingMode : uint8
    {
        kNone,
        kMSAA2x,
        kMSAA4x,
        kMSAA8x
    };
    
    // The RenderManager is really just for handling ordering and executing render Systems, presentation,
    // and maybe compute dispatches
    class RenderManager final : public NonMoveable
    {
        friend class Engine;
    public:

        IGraphicsEngine& GetGraphicsEngine() const
        {
            BASED_ASSERT(m_pGraphicsEngine, "Trying to get graphics engine when one doesn't exist!");
            return *m_pGraphicsEngine;
        }

        eAntiAliasingMode GetAntiAliasingMode() const;

    private:
        RenderManager() = default;
        ~RenderManager() = default;

        static RenderManager* Create();

        void Shutdown();

        IGraphicsEngine* m_pGraphicsEngine = nullptr;
    };
}

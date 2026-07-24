#pragma once

namespace based
{
    class IGraphicsEngine : public NonCopyable
    {
        friend class RenderManager;
    public:
        virtual ~IGraphicsEngine() = default;

        virtual void* GetGlobalInstance() const = 0;
        
        // TODO: Everything
    protected:
        IGraphicsEngine() = default;
    private:
        virtual void Initialize() = 0;
        virtual void Shutdown() = 0;
    };
}

#ifdef BASED_USE_VULKAN
#include "vulkan/VulkanGraphicsEngine.h"
#else
#error "No graphics API specified for this platform!"
#endif
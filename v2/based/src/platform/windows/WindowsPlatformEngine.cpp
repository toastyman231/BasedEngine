#include "pch.h"
#include "platform/windows/WindowsPlatformEngine.h"

#include "memory/MemoryPoolHeader.h"
#include <SDL3/SDL_timer.h>

#include "Engine.h"
#include "core/Window.h"
#include "graphics/GraphicsEngine.h"
#include "graphics/Helpers.h"

#ifdef BASED_USE_VULKAN
#include <SDL3/SDL_vulkan.h>
#endif

namespace based
{
    uint64 WindowsPlatformEngine::GetEngineTicks()
    {
        return SDL_GetTicks();
    }

    const std::string& WindowsPlatformEngine::GetPlatformName()
    {
        // Short string optimization should prevent this from making an allocation
        static std::string name("Windows");
        return name;
    }

#ifdef BASED_USE_VULKAN
    std::vector<const char*> WindowsPlatformEngine::GetVulkanInstanceExtensions()
    {
        AllocatorScope ac(ePoolIdentifier::kPersistentPool);

        std::vector<const char*> vExtensions;

#ifdef BASED_CONFIG_DEBUG
        vExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

        uint nSDLExtensionCount = 0;
        SDL_Window* pWindow = static_cast<SDL_Window*>(Engine::Instance().GetWindow()->GetNativeHandle());
        BASED_ASSERT(pWindow, "Invalid window handle! Has it been properly initialized?");
        if (!pWindow) return vExtensions;

        char const* const* ppExtensions = SDL_Vulkan_GetInstanceExtensions(&nSDLExtensionCount);
        if (nSDLExtensionCount > 0 && ppExtensions)
        {
            size_t stOldSize = vExtensions.size();
            vExtensions.resize(stOldSize + nSDLExtensionCount);
            std::copy(ppExtensions, ppExtensions + nSDLExtensionCount, vExtensions.begin() + stOldSize);
        }

        return vExtensions;
    }

    bool WindowsPlatformEngine::CanPresentToWindow(vk::PhysicalDevice device, uint32 nQueueFamily)
    {
        VkInstance instance = static_cast<VkInstance>(Engine::Instance().GetGraphicsEngine().GetGlobalInstance());
        bool bPresentationSupport = SDL_Vulkan_GetPresentationSupport(instance, device, nQueueFamily);
        check(bPresentationSupport);

        return bPresentationSupport;
    }
#endif
}

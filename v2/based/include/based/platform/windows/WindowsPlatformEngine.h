#pragma once

namespace vk {
    class PhysicalDevice;
}

namespace based
{
    class WindowsPlatformEngine final
    {
    public:
        static uint64 GetEngineTicks();
        static const std::string& GetPlatformName();
#ifdef BASED_USE_VULKAN
        static std::vector<const char*> GetVulkanInstanceExtensions();
        static bool CanPresentToWindow(vk::PhysicalDevice device, uint32 nQueueFamily);
#endif
    };

    using PlatformEngine = WindowsPlatformEngine;
}

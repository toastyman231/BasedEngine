#pragma once

#ifdef BASED_USE_VULKAN
#include <vulkan/vulkan.hpp>
#endif

namespace based
{
#ifdef BASED_USE_VULKAN
    inline void check(vk::Result result)
    {
        BASED_ASSERT_FMT(result == vk::Result::eSuccess, "Vulkan call returned an error: {}", vk::to_string(result));
    }

    inline void check(VkResult result)
    {
        BASED_ASSERT_FMT(static_cast<vk::Result>(result) == vk::Result::eSuccess, "Vulkan call returned an error: {}", 
            vk::to_string(static_cast<vk::Result>(result)));
    }
    
#endif

    inline void check(bool result)
    {
        BASED_ASSERT(result, "Call returned an error!");
    }
}

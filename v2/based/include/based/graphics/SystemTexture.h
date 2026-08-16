#pragma once

#ifdef BASED_USE_VULKAN
#include "vulkan/VulkanSystemTexture.h"
#else
#error "You need to define a SystemTexture for this platform!"
#endif

namespace based
{
#ifdef BASED_USE_VULKAN
    using SystemTexture = VulkanSystemTexture;
#else
#error "TypeDef your platform SystemTexture here!"
#endif
}

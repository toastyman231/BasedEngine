#include "pch.h"
#include "graphics/vulkan/VulkanGraphicsEngine.h"

#include "graphics/vulkan/VulkanPoolAllocator.h"
#include "memory/PlatformMemUtils.h"

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

#define VMA_IMPLEMENTATION
#if defined(__GNUC__) || defined(__clang__)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wnullability-completeness"
#endif
#include <vma/vk_mem_alloc.h>
#if defined(__GNUC__) || defined(__clang__)
    #pragma GCC diagnostic pop
#endif

#include "Engine.h"
#include "core/App.h"
#include "graphics/Helpers.h"
#include "memory/MemoryManager.h"
#include "memory/MemoryPoolHeader.h"

namespace based
{

    static void* VkAllocate(void* pUserData, size_t size, size_t alignment, vk::SystemAllocationScope allocScope)
    {
        // TODO: It might be better to set the active pool based on allocScope, but for now I won't bother
        return MemoryManager::MemAlign(size, alignment);
    }

    static void* VkReallocate(void* pUserData, void* pOriginal, size_t size, size_t alignment,
        vk::SystemAllocationScope allocScope)
    {
        return MemoryManager::MemRealloc(pOriginal, size);
    }

    static void  VkFree(void* pUserData, void* pMemory)
    {
        MemoryManager::MemFree(pMemory);
    }

    static void VkInternalAllocNotification(void* pUserData, size_t size, vk::InternalAllocationType allocType,
        vk::SystemAllocationScope allocScope)
    {
        BASED_WARN("Vulkan is internally allocating {} of type {} and scope {}", MemSize{size}, vk::to_string(allocType),
            vk::to_string(allocScope));
    }

    static void VkInternalFreeNotification(void* pUserData, size_t size, vk::InternalAllocationType allocType,
        vk::SystemAllocationScope allocScope)
    {
        BASED_WARN("Vulkan is internally freeing {} of type {} and scope {}", MemSize{size}, vk::to_string(allocType),
            vk::to_string(allocScope));
    }

    static VmaAllocator s_pAllocatorTemp = nullptr; // We store the allocator here so that the Graphics Engine can grab it later
    void SetupGraphicsPools()
    {
        auto& GE = dynamic_cast<VulkanGraphicsEngine&>(Engine::Instance().GetGraphicsEngine());
        auto pInstance = static_cast<VkInstance>(GE.GetGlobalInstance());
        auto device = GE.GetDevice();

        BASED_ASSERT(pInstance, "Invalid Vulkan instance! Did you initialize the graphics engine?");
        BASED_ASSERT(device, "Invalid Vulkan device! Did you initialize the graphics engine?");

        // TODO: Do the following, but for graphics pools.
        // We'll also need to check for available memory for each type index, and we might need to retry certain ones
        // if e.g. the user is on an older device without ReBAR support.
        
        const EngineMemoryPoolDescriptorList& poolList = GetMemoryPoolDescriptors();
        std::unordered_map<const PoolDescriptor*, std::vector<uint32>> poolMemoryTypeMap;
        const bool bSuccess = ValidateGraphicsMemoryPoolSettings(poolList, poolMemoryTypeMap);
        BASED_ASSERT(bSuccess, "Invalid graphics pool settings!");
        
        /*for (const auto& poolDescriptor : poolList.pools | std::views::values)
        {
            // Skip the invalid and root pool identifiers, and any pools that haven't been defined (for now)
            if (poolDescriptor.m_ePoolID == to_underlying(ePoolIdentifier::kInvalid)
                || poolDescriptor.m_ePoolID == to_underlying(ePoolIdentifier::kRootPool)
                || poolDescriptor.m_stPoolSize == 0) continue;
            
            size_t stPoolSize = poolDescriptor.m_stPoolSize;
            ePoolIdentifier eParentPoolID = static_cast<ePoolIdentifier>(poolDescriptor.m_eParentPoolID);

            void* pBackingMem = nullptr;

            // Since VMA requires Vulkan to be set up, we're forced to setup GPU mem pools separately on that
            // platform. Therefore, all platforms set up GPU mempools separately. 
            if (eParentPoolID != ePoolIdentifier::kInvalid)
            {
                AllocatorScope ac(eParentPoolID);
                pBackingMem = new uint8[stPoolSize + sizeof(MemoryPoolHeader) + sizeof(MemPoolTLSFAllocator)];
            } else
            {
                pBackingMem = AllocateSystemMemory(
                    stPoolSize + sizeof(MemoryPoolHeader) + sizeof(MemPoolTLSFAllocator));
            }
            
            BASED_ASSERT(pBackingMem, "Invalid memory allocated!");
            MemoryPoolHeader::CreatePool(poolDescriptor.m_strPoolName,
                static_cast<ePoolIdentifier>(poolDescriptor.m_ePoolID),
                stPoolSize, pBackingMem);
        }*/
    }
    
    size_t GetTotalGraphicsMemoryBytes()
    {
        vk::PhysicalDeviceMemoryBudgetPropertiesEXT budgetProperties{};

        vk::PhysicalDeviceMemoryProperties2 memProperties2{};
        memProperties2.pNext = &budgetProperties;
        
        auto& GE = dynamic_cast<VulkanGraphicsEngine&>(Engine::Instance().GetGraphicsEngine());
        GE.GetPhysicalDevice().getMemoryProperties2(&memProperties2);

        vk::DeviceSize nTotalMemory = 0;
        for (uint32_t i = 0; i < memProperties2.memoryProperties.memoryHeapCount; ++i) 
        {
            if (memProperties2.memoryProperties.memoryHeaps[i].flags & vk::MemoryHeapFlagBits::eDeviceLocal) 
            {
                nTotalMemory += memProperties2.memoryProperties.memoryHeaps[i].size;
            }
        }

        return nTotalMemory;
    }
    
    size_t GetAvailableGraphicsMemoryBytes()
    {
        vk::PhysicalDeviceMemoryBudgetPropertiesEXT budgetProperties{};

        vk::PhysicalDeviceMemoryProperties2 memProperties2{};
        memProperties2.pNext = &budgetProperties;
        
        auto& GE = dynamic_cast<VulkanGraphicsEngine&>(Engine::Instance().GetGraphicsEngine());
        GE.GetPhysicalDevice().getMemoryProperties2(&memProperties2);

        vk::DeviceSize nAvailableMemory = 0;
        for (uint32_t i = 0; i < memProperties2.memoryProperties.memoryHeapCount; ++i) 
        {
            if (memProperties2.memoryProperties.memoryHeaps[i].flags & vk::MemoryHeapFlagBits::eDeviceLocal) 
            {
                vk::DeviceSize osBudget = budgetProperties.heapBudget[i];
                vk::DeviceSize currentUsage = budgetProperties.heapUsage[i];
                nAvailableMemory += osBudget - currentUsage;
            }
        }
        
        return nAvailableMemory;
    }

    std::vector<uint32> GetAcceptableGPUMemoryTypes(eGPUMemoryRequirements eRequirements)
    {
        constexpr uint32 kMaxAcceptableMemTypes = 3;
        BASED_ASSERT(
            !HasBit(eRequirements, eGPUMemoryRequirements::kCPUVisibleRequired & eGPUMemoryRequirements::kCPUVisiblePreferred), 
                "Memory requirements cannot both require and prefer CPU visibility, pick one!");
        BASED_ASSERT(
            !HasBit(eRequirements, eGPUMemoryRequirements::kDeviceLocalRequired & eGPUMemoryRequirements::kDeviceLocalPreferred),
                "Memory requirements cannot both require and prefer device local, pick one!");
        BASED_ASSERT(
            !HasBit(eRequirements, eGPUMemoryRequirements::kCPUReadbackRequired & eGPUMemoryRequirements::kCPUReadbackPreferred),
                "Memory requirements cannot both require and prefer CPU readback, pick one!");
        
        std::array<vk::MemoryPropertyFlags, kMaxAcceptableMemTypes> vAcceptableMemTypeFlags{};
        std::vector<uint32> vAcceptableMemTypes;
        
        // If we require host visibility, try to get it to be coherent, otherwise fallback to visible only
        // If host visibility is only preferred, then we do the same, except we allow the final fallback slot to not have that requirement
        if (HasBit(eRequirements, eGPUMemoryRequirements::kCPUVisibleRequired))
        {
            BASED_ASSERT(!HasBit(eRequirements, eGPUMemoryRequirements::kNoCPU), "If CPU Visibility is required, you cannot have the NoCPU flag!");
            vAcceptableMemTypeFlags[0] |= vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
            vAcceptableMemTypeFlags[1] |= vk::MemoryPropertyFlagBits::eHostVisible;
            vAcceptableMemTypeFlags[2] |= vk::MemoryPropertyFlagBits::eHostVisible;
        } else if (HasBit(eRequirements, eGPUMemoryRequirements::kCPUVisiblePreferred))
        {
            BASED_ASSERT(!HasBit(eRequirements, eGPUMemoryRequirements::kNoCPU), "If CPU Visibility is preferred, you shouldn't have the NoCPU flag!");
            vAcceptableMemTypeFlags[0] |= vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
            vAcceptableMemTypeFlags[1] |= vk::MemoryPropertyFlagBits::eHostVisible;
        }
        
        // Mostly the same as above, except there's only one flag we care about here
        if (HasBit(eRequirements, eGPUMemoryRequirements::kDeviceLocalRequired))
        {
            BASED_ASSERT(!HasBit(eRequirements, eGPUMemoryRequirements::kNoGPU), "If Device Local is required, you cannot have the NoGPU flag!");
            vAcceptableMemTypeFlags[0] |= vk::MemoryPropertyFlagBits::eDeviceLocal;
            vAcceptableMemTypeFlags[1] |= vk::MemoryPropertyFlagBits::eDeviceLocal;
            vAcceptableMemTypeFlags[2] |= vk::MemoryPropertyFlagBits::eDeviceLocal;
        } else if (HasBit(eRequirements, eGPUMemoryRequirements::kDeviceLocalPreferred))
        {
            BASED_ASSERT(!HasBit(eRequirements, eGPUMemoryRequirements::kNoGPU), "If Device Local is required, you shouldn't have the NoGPU flag!");
            vAcceptableMemTypeFlags[0] |= vk::MemoryPropertyFlagBits::eDeviceLocal;
            vAcceptableMemTypeFlags[1] |= vk::MemoryPropertyFlagBits::eDeviceLocal;
        }
        
        // Tier 0 will evaluate to VISIBLE | COHERENT | CACHED, which I don't think most modern desktop GPUs support
        // This is okay because the index evaluation will simply fail and be skipped, and we'll fall back to tier 1, which
        // will only be VISIBLE | CACHED, which should be supported
        if (HasBit(eRequirements, eGPUMemoryRequirements::kCPUReadbackRequired))
        {
            BASED_ASSERT(!HasBit(eRequirements, eGPUMemoryRequirements::kNoCPU), "If CPU Visibility is required, you cannot have the NoCPU flag!");
            vAcceptableMemTypeFlags[0] |= vk::MemoryPropertyFlagBits::eHostCached;
            vAcceptableMemTypeFlags[1] |= vk::MemoryPropertyFlagBits::eHostCached;
            vAcceptableMemTypeFlags[2] |= vk::MemoryPropertyFlagBits::eHostCached;
        } else if (HasBit(eRequirements, eGPUMemoryRequirements::kCPUReadbackPreferred))
        {
            BASED_ASSERT(!HasBit(eRequirements, eGPUMemoryRequirements::kNoCPU), "If CPU Visibility is preferred, you shouldn't have the NoCPU flag!");
            vAcceptableMemTypeFlags[0] |= vk::MemoryPropertyFlagBits::eHostCached;
            vAcceptableMemTypeFlags[1] |= vk::MemoryPropertyFlagBits::eHostCached;
        }
        
        for (uint32 i = 0; i < kMaxAcceptableMemTypes; ++i)
        {
            if (vAcceptableMemTypeFlags[i] != static_cast<vk::MemoryPropertyFlags>(0))
            {
                uint32 index = VulkanPoolAllocator::FindMemoryTypeIndex(vAcceptableMemTypeFlags[i]);
                if (index != kInvalidMemoryTypeIndex 
                    && std::ranges::find(vAcceptableMemTypes, index) == vAcceptableMemTypes.end())
                {
                    vAcceptableMemTypes.emplace_back(index);
                }
            }
        }
        
        return vAcceptableMemTypes;
    }

    bool ValidateGraphicsMemoryPoolSettings(const EngineMemoryPoolDescriptorList& poolList, 
        std::unordered_map<const PoolDescriptor*, std::vector<uint32>>& outPoolMemoryTypeMap)
    {
        // Resolve pool memory type map
        for (const auto& poolDescriptor : poolList.pools | std::views::values)
        {
            if (poolDescriptor.m_ePoolID == to_underlying(ePoolIdentifier::kRootPool)
                || !poolDescriptor.m_bIsGPUPool) continue;
            
            BASED_ASSERT(poolDescriptor.m_ePoolID != to_underlying(ePoolIdentifier::kInvalid),
                "Pool descriptor has invalid ID, did you define all the engine pools?");
            if (poolDescriptor.m_ePoolID == to_underlying(ePoolIdentifier::kInvalid)) return false;
            
            outPoolMemoryTypeMap.emplace(&poolDescriptor, GetAcceptableGPUMemoryTypes(poolDescriptor.m_eGPUMemRequirements));
            
            size_t stPoolSize = poolDescriptor.m_stPoolSize;
            const std::string_view strPoolName = poolDescriptor.m_strPoolName;
            BASED_ASSERT_FMT(stPoolSize > 0, "Invalid size {} for pool {}, {}!", stPoolSize, poolDescriptor.m_ePoolID, strPoolName);
            if (stPoolSize <= 0) return false;
        }
        
        // Multiple memory type indices can use the same heap, so we group our pools by the heap they fall into when
        // checking if there's enough memory for our pools
        std::unordered_map<uint32, size_t> sizesByHeapMap;
        auto& GE = dynamic_cast<VulkanGraphicsEngine&>(Engine::Instance().GetGraphicsEngine());
        auto device = GE.GetPhysicalDevice();
        
        vk::PhysicalDeviceMemoryProperties memProps;
        device.getMemoryProperties(&memProps);
        
        // Group pools by tier 0 (ideal memory type index) and sum pool totals
        for (const auto& poolDescriptor : poolList.pools | std::views::values)
        {
            if (poolDescriptor.m_ePoolID == to_underlying(ePoolIdentifier::kRootPool)
                || !poolDescriptor.m_bIsGPUPool) continue;
            
            uint32 nIdealMemTypeIndex = outPoolMemoryTypeMap[&poolDescriptor][0];
            uint32 nHeapIndex = memProps.memoryTypes[nIdealMemTypeIndex].heapIndex;
            
            if (sizesByHeapMap.contains(nHeapIndex))
            {
                sizesByHeapMap[nHeapIndex] += poolDescriptor.m_stPoolSize;
            } else
            {
                sizesByHeapMap.emplace(nHeapIndex, poolDescriptor.m_stPoolSize);
            }
        }
        
        // Get device properties again here, in case they've changed
        device.getMemoryProperties(&memProps);
        
        // Ensure heaps have enough memory for the ideal config
        for (const auto& [nHeapIndex, size] : sizesByHeapMap)
        {
            vk::DeviceSize nHeapSize = memProps.memoryHeaps[nHeapIndex].size;
            BASED_ASSERT_FMT(nHeapSize > size, "Not enough memory in heap {}! Want: {}, have: {}",
                nHeapIndex, MemSize{size}, MemSize{nHeapSize});
            if (nHeapSize <= size) return false;
        }

        return true;
    }
    
    const vk::AllocationCallbacks* VulkanGraphicsEngine::GetAllocationCallbacks()
    {
        static vk::AllocationCallbacks allocCallbacks;
        static bool bDoOnce = false;
        if (!bDoOnce)
        {
            allocCallbacks.setPfnAllocation(VkAllocate)
                        .setPfnReallocation(VkReallocate)
                        .setPfnFree(VkFree)
                        .setPfnInternalAllocation(VkInternalAllocNotification)
                        .setPfnInternalFree(VkInternalFreeNotification);
            bDoOnce = true;
        }

        return &allocCallbacks;
    }

    static void SetupVMA()
    {
        auto& GE = dynamic_cast<VulkanGraphicsEngine&>(Engine::Instance().GetGraphicsEngine());
        auto pInstance = static_cast<VkInstance>(GE.GetGlobalInstance());
        auto device = GE.GetDevice();

        BASED_ASSERT(pInstance, "Invalid Vulkan instance! Did you initialize the graphics engine?");
        BASED_ASSERT(device, "Invalid Vulkan device! Did you initialize the graphics engine?");
        
        VmaVulkanFunctions vkFunctions{};
        vkFunctions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
        vkFunctions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;
        VmaAllocatorCreateInfo allocatorCI{
            .flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
            .physicalDevice = GE.GetPhysicalDevice(),
            .device = device,
            .pAllocationCallbacks = *VulkanGraphicsEngine::GetAllocationCallbacks(),
            .pVulkanFunctions = &vkFunctions,
            .instance = pInstance
        };
        check(vmaCreateAllocator(&allocatorCI, &s_pAllocatorTemp));
    }

    void VulkanGraphicsEngine::Initialize()
    {
        AllocatorScope ac(ePoolIdentifier::kPersistentPool/*, ePoolIdentifier::kPersistentGraphicsPool*/);

        VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);

        // Get Based app info
        AppInfo basedAppInfo = Engine::Instance().GetApp().GetAppInfo();
        WindowProperties windowProps = Engine::Instance().GetApp().GetWindowProperties();
        const char* pAppName = nullptr;
        if (!basedAppInfo.appName.empty())
        {
            pAppName = basedAppInfo.appName.data();
        } else if (!windowProps.title.empty())
        {
            pAppName = windowProps.title.data();
        } else
        {
            pAppName = "Based App";
        }

        // Instance setup
        vk::ApplicationInfo appInfo;
        appInfo.setPApplicationName(pAppName)
                .setApiVersion(VK_API_VERSION_1_3)
                .setPEngineName("Based Engine");

        std::vector<const char*> vExtensions = PlatformEngine::GetVulkanInstanceExtensions();

        vk::InstanceCreateInfo instanceCI;
        instanceCI.setPApplicationInfo(&appInfo)
                    .setEnabledExtensionCount(vExtensions.size())
                    .setPpEnabledExtensionNames(vExtensions.data());
        check(vk::createInstance(&instanceCI, GetAllocationCallbacks(), &m_Instance));
        volkLoadInstance(m_Instance);
        VULKAN_HPP_DEFAULT_DISPATCHER.init(m_Instance);

        // Physical device setup
        uint32 nDeviceCount = 0;
        check(m_Instance.enumeratePhysicalDevices(&nDeviceCount, nullptr));
        std::vector<vk::PhysicalDevice> vDevices(nDeviceCount);
        check(m_Instance.enumeratePhysicalDevices(&nDeviceCount, vDevices.data()));
        
        uint32 nDeviceIndex = 0;
        for (uint i = 0; i < nDeviceCount; ++i)
        {
            vk::PhysicalDeviceProperties2 deviceProperties{};
            vDevices[i].getProperties2(&deviceProperties);
            if (deviceProperties.properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
            {
                nDeviceIndex = i;
                break;
            }
        }
        m_PhysicalDevice = vDevices[nDeviceIndex];
        
        vk::PhysicalDeviceProperties2 deviceProperties{};
        m_PhysicalDevice.getProperties2(&deviceProperties);
        BASED_INFO("Selected device: {}", deviceProperties.properties.deviceName.data());

        // Queue setup
        uint32 nQueueFamilyCount = 0;
        m_PhysicalDevice.getQueueFamilyProperties(&nQueueFamilyCount, nullptr);
        std::vector<vk::QueueFamilyProperties> vQueueFamProperties(nQueueFamilyCount);
        m_PhysicalDevice.getQueueFamilyProperties(&nQueueFamilyCount, vQueueFamProperties.data());

        uint32 nQueueFamily = 0;
        for (uint i = 0; i < vQueueFamProperties.size(); ++i)
        {
            if (vQueueFamProperties[i].queueFlags & vk::QueueFlagBits::eGraphics
                && vQueueFamProperties[i].queueFlags & vk::QueueFlagBits::eCompute)
            {
                nQueueFamily = i;
                break;
            }
        }
        BASED_ASSERT(PlatformEngine::CanPresentToWindow(m_PhysicalDevice, nQueueFamily), "Platform can't present to window!");

        // Logical device setup
        const float fQueuePriorities = 1.0f;
        vk::DeviceQueueCreateInfo queueCI;
        queueCI.setQueueFamilyIndex(nQueueFamily)
                .setQueueCount(1)
                .setPQueuePriorities(&fQueuePriorities);

        const std::vector<const char*> vDeviceExtensions{ vk::KHRSwapchainExtensionName, vk::EXTMemoryBudgetExtensionName };
        vk::PhysicalDeviceVulkan12Features enabledVk12Features;
        enabledVk12Features.setDescriptorIndexing(true)
                            .setShaderSampledImageArrayNonUniformIndexing(true)
                            .setDescriptorBindingVariableDescriptorCount(true)
                            .setRuntimeDescriptorArray(true)
                            .setBufferDeviceAddress(true);
        vk::PhysicalDeviceVulkan13Features enabledVk13Features;
        enabledVk13Features.setPNext(&enabledVk12Features)
                            .setSynchronization2(true)
                            .setDynamicRendering(true);
        vk::PhysicalDeviceFeatures enabledVk10Features;
        enabledVk10Features.setSamplerAnisotropy(VK_TRUE);

        vk::DeviceCreateInfo deviceCI;
        deviceCI.setPNext(&enabledVk13Features)
                .setQueueCreateInfoCount(1)
                .setPQueueCreateInfos(&queueCI)
                .setEnabledExtensionCount(static_cast<uint32>(vDeviceExtensions.size()))
                .setPpEnabledExtensionNames(vDeviceExtensions.data())
                .setPEnabledFeatures(&enabledVk10Features);
        check(m_PhysicalDevice.createDevice(&deviceCI, nullptr, &m_Device));\
        volkLoadDevice(m_Device);
        VULKAN_HPP_DEFAULT_DISPATCHER.init(m_Device);

        m_Device.getQueue(nQueueFamily, 0, &m_Queue);

        // Setup VMA and mem pools
        SetupVMA();
        m_Allocator = s_pAllocatorTemp;
        SetupGraphicsPools();

        // Setup surface
        IWindow* pWindow = Engine::Instance().GetWindow();
        VkSurfaceKHR pSurface = static_cast<VkSurfaceKHR>(WindowFactory::CreateAndGetWindowSurface(pWindow));
        m_Surface = pSurface;

        vk::SurfaceCapabilitiesKHR surfaceCaps{};
        check(m_PhysicalDevice.getSurfaceCapabilitiesKHR(m_Surface, &surfaceCaps));

        // Setup swapchain
        vk::Extent2D swapchainExtent(surfaceCaps.currentExtent);
        if (surfaceCaps.currentExtent.width == 0xFFFFFFFF) {
            // Wayland just returns 0xFFFFFFFF so we have to set this up manually in that case
            swapchainExtent = vk::Extent2D(
                static_cast<uint32_t>(pWindow->GetSize().x),
                static_cast<uint32_t>(pWindow->GetSize().y)
            );
        }

        constexpr vk::Format imageFormat = vk::Format::eB8G8R8A8Srgb;
        vk::SwapchainCreateInfoKHR swapchainCI;
        swapchainCI.setSurface(m_Surface)
                    .setMinImageCount(surfaceCaps.minImageCount)
                    .setImageFormat(imageFormat)
                    .setImageColorSpace(vk::ColorSpaceKHR::eSrgbNonlinear)
                    .setImageExtent(vk::Extent2D(swapchainExtent.width, swapchainExtent.height))
                    .setImageArrayLayers(1)
                    .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
                    .setPreTransform(vk::SurfaceTransformFlagBitsKHR::eIdentity)
                    .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
                    .setPresentMode(vk::PresentModeKHR::eFifo);
        check(m_Device.createSwapchainKHR(&swapchainCI, GetAllocationCallbacks(), &m_Swapchain));

        uint32 nImageCount = 0;
        check(m_Device.getSwapchainImagesKHR(m_Swapchain, &nImageCount, nullptr));
        m_vSwapchainImages.resize(nImageCount);
        check(m_Device.getSwapchainImagesKHR(m_Swapchain, &nImageCount, m_vSwapchainImages.data()));
        m_vSwapchainImageViews.resize(nImageCount);
    }
    
    void VulkanGraphicsEngine::Shutdown() {}
}

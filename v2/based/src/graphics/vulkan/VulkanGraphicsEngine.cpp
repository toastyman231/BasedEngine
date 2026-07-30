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

        s_nHostCachedOrDeviceIndex = VulkanPoolAllocator::FindMemoryTypeIndex(
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
        vk::MemoryPropertyFlagBits::eHostCached | vk::MemoryPropertyFlagBits::eDeviceLocal);

        s_nHostCachedOnlyIndex = VulkanPoolAllocator::FindMemoryTypeIndex(
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
            vk::MemoryPropertyFlagBits::eHostCached);

        s_nHostCoherentOrDeviceIndex = VulkanPoolAllocator::FindMemoryTypeIndex(
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
            vk::MemoryPropertyFlagBits::eDeviceLocal);

        s_nHostCoherentOnlyIndex = VulkanPoolAllocator::FindMemoryTypeIndex(
            vk::MemoryPropertyFlagBits::eHostVisible,
            vk::MemoryPropertyFlagBits::eHostCoherent);

        s_nDeviceLocalOnlyIndex = VulkanPoolAllocator::FindMemoryTypeIndex(
            vk::MemoryPropertyFlagBits::eDeviceLocal, vk::MemoryPropertyFlagBits::eDeviceLocal);

        s_nDeviceLocalOrHostVisibleIndex = VulkanPoolAllocator::FindMemoryTypeIndex(
                    vk::MemoryPropertyFlagBits::eDeviceLocal,
                    vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);   

        // TODO: Do the following, but for graphics pools.
        // We'll also need to check for available memory for each type index, and we might need to retry certain ones
        // if e.g. the user is on an older device without ReBAR support.
        
        /*const EngineMemoryPoolDescriptorList& poolList = GetMemoryPoolDescriptors();
#ifdef BASED_CONFIG_DEBUG
        const bool bSuccess = ValidateMemoryPoolSettings(poolList);
        BASED_ASSERT(bSuccess, "Invalid pool settings!");
#endif
        
        for (const auto& poolDescriptor : poolList.pools | std::views::values)
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
    
    bool ValidateGraphicsMemoryPoolSettings(const EngineMemoryPoolDescriptorList& poolList)
    {
        return false;
        /*size_t stRequestedDeviceLocal = 0;
        for (const auto& poolDescriptor : poolList.pools | std::views::values)
        {
            // Skip the invalid and root pool identifiers, and GPU pools since we set those up separately
            if (poolDescriptor.m_ePoolID == to_underlying(ePoolIdentifier::kInvalid)
                || poolDescriptor.m_ePoolID == to_underlying(ePoolIdentifier::kRootPool)
                || poolDescriptor.m_bIsGPUPool) continue;
            
            size_t stPoolSize = poolDescriptor.m_stPoolSize;
            const std::string_view strPoolName = poolDescriptor.m_strPoolName;
            BASED_ASSERT_FMT(stPoolSize > 0, "Invalid size {} for pool {}!", stPoolSize, poolDescriptor.m_ePoolID);
            if (stPoolSize <= 0) continue; // TODO: Once I have all the pools sorted, this should return failure

            ePoolIdentifier eParentPoolID = static_cast<ePoolIdentifier>(poolDescriptor.m_eParentPoolID);
            if (eParentPoolID != ePoolIdentifier::kInvalid)
            {
                const auto pParentDescriptor = poolList.Find(eParentPoolID);
                BASED_ASSERT_FMT(pParentDescriptor, "No pool descriptor found for pool {}!", poolDescriptor.m_eParentPoolID);
                if (!pParentDescriptor) return false;
                
                BASED_ASSERT_FMT(stPoolSize < pParentDescriptor->m_stPoolSize,
                        "Child pool {} ({}) is bigger than parent {} ({})!", strPoolName, MemSize{stPoolSize},
                            pParentDescriptor->m_strPoolName, MemSize{pParentDescriptor->m_stPoolSize});
                if (stPoolSize >= pParentDescriptor->m_stPoolSize) return false;
            } else
            {
                totalSize += stPoolSize;
            }
        }

#ifndef SKIP_AVAILABLE_MEMORY_CHECK
        size_t totalRAM = GetTotalSystemMemoryBytes();
        size_t availRAM = GetAvailableSystemMemoryBytes();
        BASED_ASSERT_FMT(totalSize <= totalRAM && totalSize <= availRAM,
            "System does not have enough memory! Want: {}, Have: {}, Available: {}", totalSize, totalRAM, availRAM);
        if (totalSize > totalRAM || totalSize > availRAM) return false;
#else
        size_t totalRAM = GetTotalSystemMemoryBytes();
        BASED_ASSERT_FMT(totalSize <= totalRAM,
            "System does not have enough memory! Want: {}, Have: {}", totalSize, totalRAM);
        if (totalSize > totalRAM) return false;
#endif

        return true;*/
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

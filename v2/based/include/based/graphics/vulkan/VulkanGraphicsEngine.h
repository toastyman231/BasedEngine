#pragma once

#include <Volk/volk.h>
#include <vulkan/vulkan.hpp>
#include <vma/vk_mem_alloc.h>

#include "graphics/GraphicsEngine.h"

namespace based
{
    constexpr uint32 kInvalidMemoryTypeIndex = std::numeric_limits<uint32>::max();
    
    class VulkanGraphicsEngine final : public IGraphicsEngine
    {
        friend class RenderManager;
        friend class VulkanPoolAllocator;
    public:

        void* GetGlobalInstance() const override { return m_Instance; }

        vk::Device GetDevice() const { return m_Device; }
        vk::PhysicalDevice GetPhysicalDevice() const { return m_PhysicalDevice; }
        
        static const vk::AllocationCallbacks* GetAllocationCallbacks();

    private:
        VulkanGraphicsEngine() = default;
        void Initialize() override;
        void Shutdown() override;

        VmaAllocator m_Allocator = nullptr;

        vk::Instance m_Instance = nullptr;
        vk::PhysicalDevice m_PhysicalDevice = nullptr;
        vk::Device m_Device = nullptr;
        vk::Queue m_Queue = nullptr;
        vk::SurfaceKHR m_Surface = nullptr;
        vk::SwapchainKHR m_Swapchain = nullptr;

        std::vector<vk::Image> m_vSwapchainImages;
        std::vector<vk::ImageView> m_vSwapchainImageViews;
    };

    using PlatformGraphicsEngine = VulkanGraphicsEngine;
}

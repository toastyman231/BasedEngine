#pragma once
#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>

#include "BasedDefines.h"
#include "memory/MemoryPoolAllocator.h"
#include "memory/MemoryPoolHeader.h"

namespace based
{
    class VulkanPoolAllocator final : public IMemoryPoolAllocator
    {
    public:
        enum class eVulkanPoolUsage : uint8
        {
            kNone,
            kUseLinearAlgorithm = 1 << 0,
            kAllBufferUsage     = 1 << 1
        };

        struct Header final
        {
            VmaVirtualAllocation m_Alloc;
            size_t m_Size;
            IMemoryPoolAllocator* m_pAllocator;

            Header(VmaVirtualAllocation alloc, size_t size, IMemoryPoolAllocator* pAllocator)
                : m_Alloc(alloc)
                , m_Size(size)
                , m_pAllocator(pAllocator)
            {}
        };
        
        // nMemoryTypeIndex should be one of the values defined below
        void Initialize(const PoolDescriptor& poolDescriptor, uint32 nMemoryTypeIndex, eVulkanPoolUsage flags);
        void Shutdown();
        
        [[nodiscard]] void* Allocate(size_t bytes) override;
        [[nodiscard]] void* Allocate(size_t size, size_t alignment) override;
        [[nodiscard]] void* Reallocate(void* ptr, size_t size) override;
        void Deallocate(void* ptr) override;
        [[nodiscard]] bool IsPointerFromAllocator(void* ptr) const override;
        PoolStats GetPoolStats(MemoryPoolHeader* pHeader) override;

        static uint32 FindMemoryTypeIndex(vk::MemoryPropertyFlags required, vk::MemoryPropertyFlags preferred);
        std::optional<const VulkanPoolAllocator::Header*> FindHeader(void* ptr) const;

    private:
        VmaPool m_Pool = nullptr;
        VmaVirtualBlock m_VirtualBlock = nullptr;
        vk::DeviceMemory m_GPUBaseAddress = nullptr;
        vk::DeviceSize m_GPUOffset = 0;
        void* m_pCPUBaseAddress = nullptr;
        VmaAllocation m_BaseAllocation = nullptr;
        eVulkanPoolUsage m_CreationFlags = eVulkanPoolUsage::kNone;
        bool m_bHostMapped = false;

        std::unordered_map<void*, Header> m_AllocationMap;
    };

    static uint32 s_nHostCachedOrDeviceIndex = 0;
    static uint32 s_nHostCachedOnlyIndex = 0;
    static uint32 s_nHostCoherentOrDeviceIndex = 0;
    static uint32 s_nHostCoherentOnlyIndex = 0;
    static uint32 s_nDeviceLocalOnlyIndex = 0;
    static uint32 s_nDeviceLocalOrHostVisibleIndex = 0;

    DEFINE_ENUM_CLASS_BITWISE_OPERATORS(VulkanPoolAllocator::eVulkanPoolUsage)
}

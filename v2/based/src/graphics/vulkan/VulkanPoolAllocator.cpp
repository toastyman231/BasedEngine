#include "pch.h"
#include "graphics/vulkan/VulkanPoolAllocator.h"
#include "graphics/vulkan/VulkanPoolAllocator.h"

#include <vma/vk_mem_alloc.h>

#include <algorithm>

#include "Engine.h"
#include "graphics/Helpers.h"
#include "graphics/vulkan/VulkanGraphicsEngine.h"
#include "memory/PlatformMemUtils.h"

static constexpr size_t kVulkanMaxImageAlignment = 4 * 1024 * 1024;
static constexpr size_t kVulkanMaxBufferAlignment = 256;
static constexpr size_t kVulkanDefaultBufferAlignment = 16;

namespace based
{
    uint32 VulkanPoolAllocator::FindMemoryTypeIndex(vk::MemoryPropertyFlags required, vk::MemoryPropertyFlags preferred)
    {
        static VulkanGraphicsEngine& GE = dynamic_cast<VulkanGraphicsEngine&>(Engine::Instance().GetGraphicsEngine());
        
        VmaAllocationCreateInfo ci{};
        ci.requiredFlags = static_cast<VkMemoryPropertyFlags>(required);
        ci.preferredFlags = static_cast<VkMemoryPropertyFlags>(preferred);
        uint32 index;
        check(vmaFindMemoryTypeIndex(GE.m_Allocator, ~0u, &ci, &index));

        return index;
    }

    std::optional<const VulkanPoolAllocator::Header*> VulkanPoolAllocator::FindHeader(void* ptr) const
    {
        auto it = m_AllocationMap.find(ptr);
        if (it == m_AllocationMap.end()) BASED_ERROR("Could not find header for ptr {}!", ptr);

        return &it->second;
    }

    void VulkanPoolAllocator::Initialize(const PoolDescriptor& poolDescriptor, uint32 nMemoryTypeIndex, eVulkanPoolUsage flags)
    {
        AllocatorScope ac(ePoolIdentifier::kPersistentPool);
        
        static VulkanGraphicsEngine& GE = dynamic_cast<VulkanGraphicsEngine&>(Engine::Instance().GetGraphicsEngine());
        m_CreationFlags = flags;
        
        VmaPoolCreateInfo poolCI{
            .memoryTypeIndex = nMemoryTypeIndex,
            .blockSize = poolDescriptor.m_stPoolSize,
            .minBlockCount = 1,
            .maxBlockCount = 1
        };

        check(vmaCreatePool(GE.m_Allocator, &poolCI, &m_Pool));

        VkMemoryRequirements reqs;
        reqs.size = poolDescriptor.m_stPoolSize,
        reqs.alignment = (flags & eVulkanPoolUsage::kAllBufferUsage) != eVulkanPoolUsage::kNone
                    ? kVulkanMaxBufferAlignment
                    : kVulkanMaxImageAlignment;
        reqs.memoryTypeBits = 1u << nMemoryTypeIndex;
        VmaAllocationCreateInfo allocCI{};
        allocCI.pool = m_Pool;
        allocCI.flags = nMemoryTypeIndex != s_nDeviceLocalIndex ? VMA_ALLOCATION_CREATE_MAPPED_BIT : 0;
        
        VmaAllocationInfo blockInfo;
        check(vmaAllocateMemory(GE.m_Allocator, &reqs, &allocCI, &m_BaseAllocation, &blockInfo));

        m_GPUBaseAddress = blockInfo.deviceMemory;
        m_GPUOffset = blockInfo.offset;
        m_pCPUBaseAddress = blockInfo.pMappedData;
        m_bHostMapped = m_pCPUBaseAddress != nullptr;

        if (!m_pCPUBaseAddress)
        {
            m_pCPUBaseAddress = AllocateSystemMemory(poolDescriptor.m_stPoolSize, true, false, false);
        }
        
        // If this pool can be used for buffers, then bind a megabuffer with every kind of usage,
        // which we will parcel out offsets into for our individual buffers
        if ((flags & eVulkanPoolUsage::kAllBufferUsage) != eVulkanPoolUsage::kNone)
        {
            vk::BufferUsageFlags allUsages =
                  vk::BufferUsageFlagBits::eVertexBuffer
                | vk::BufferUsageFlagBits::eIndexBuffer
                | vk::BufferUsageFlagBits::eUniformBuffer
                | vk::BufferUsageFlagBits::eStorageBuffer
                | vk::BufferUsageFlagBits::eIndirectBuffer
                | vk::BufferUsageFlagBits::eTransferSrc
                | vk::BufferUsageFlagBits::eTransferDst
                | vk::BufferUsageFlagBits::eUniformTexelBuffer
                | vk::BufferUsageFlagBits::eStorageTexelBuffer
                | vk::BufferUsageFlagBits::eShaderDeviceAddress;

            vk::BufferCreateInfo bufferCI{};
            bufferCI.size = poolDescriptor.m_stPoolSize;
            bufferCI.usage = allUsages;
            bufferCI.sharingMode = vk::SharingMode::eExclusive;

            vk::Buffer buffer;
            check(GE.GetDevice().createBuffer(&bufferCI,
                VulkanGraphicsEngine::GetAllocationCallbacks(), &buffer));

            GE.GetDevice().bindBufferMemory(buffer, m_GPUBaseAddress, blockInfo.offset);
        }

        // Everything, even textures is actually just an offset into a big pre-allocated buffer
        VmaVirtualBlockCreateInfo virtualBlockCI{
            .size = poolDescriptor.m_stPoolSize
        };
        virtualBlockCI.flags = (flags & eVulkanPoolUsage::kUseLinearAlgorithm) != eVulkanPoolUsage::kNone
                                    ? VMA_POOL_CREATE_LINEAR_ALGORITHM_BIT
                                    : 0;
        check(vmaCreateVirtualBlock(&virtualBlockCI, &m_VirtualBlock));
    }
    
    void VulkanPoolAllocator::Shutdown()
    {
        AllocatorScope ac(ePoolIdentifier::kPersistentPool);
        
        VulkanGraphicsEngine& GE = dynamic_cast<VulkanGraphicsEngine&>(Engine::Instance().GetGraphicsEngine());
        
        // Because all of our pools are actually megabuffers, we can just free the base address before destroying the pool
        vmaFreeMemory(GE.m_Allocator, m_BaseAllocation);
        vmaDestroyPool(GE.m_Allocator, m_Pool);
    }
    
    void* VulkanPoolAllocator::Allocate(size_t bytes)
    {
        BASED_ASSERT((m_CreationFlags & eVulkanPoolUsage::kAllBufferUsage) == eVulkanPoolUsage::kNone,
            "You must specify an alignment if you're allocating something other than a buffer!");

        return Allocate(bytes, kVulkanDefaultBufferAlignment);
    }
    
    void* VulkanPoolAllocator::Allocate(size_t size, size_t alignment)
    {
        AllocatorScope ac(ePoolIdentifier::kPersistentPool);
        
        VmaVirtualAllocationCreateInfo allocCI{
            .size = size,
            .alignment = alignment
        };
        VmaVirtualAllocation vAlloc;
        vk::DeviceSize offset;
        check(vmaVirtualAllocate(m_VirtualBlock, &allocCI, &vAlloc, &offset));

        void* result = static_cast<char*>(m_pCPUBaseAddress) + offset;
        auto [it, bSuccess] = m_AllocationMap.try_emplace(result, vAlloc, size, this);
        BASED_ASSERT(bSuccess, "Error putting allocation at offset {} into header map!", vk::to_string(offset));

        return result;
    }
    
    void* VulkanPoolAllocator::Reallocate(void* ptr, size_t size)
    {
        BASED_ASSERT(false, "Don't reallocate GPU objects!");
        return nullptr;
    }
    
    void VulkanPoolAllocator::Deallocate(void* ptr)
    {
        AllocatorScope ac(ePoolIdentifier::kPersistentPool);
        
        auto maybeHeader = FindHeader(ptr);
        BASED_ASSERT(maybeHeader.has_value(), "Header could not be found for pointer {}!", ptr);

        const Header* pHeader = maybeHeader.value();
        vmaVirtualFree(m_VirtualBlock, pHeader->m_Alloc);
        m_AllocationMap.erase(ptr);
    }
    
    bool VulkanPoolAllocator::IsPointerFromAllocator(void* ptr) const
    {
        return m_AllocationMap.contains(ptr);
    }
    
    PoolStats VulkanPoolAllocator::GetPoolStats(MemoryPoolHeader* pHeader)
    {
        AllocatorScope ac(ePoolIdentifier::kPersistentPool);

        VmaDetailedStatistics stats;
        vmaCalculateVirtualBlockStatistics(m_VirtualBlock, &stats);

        static PoolStats poolStats;

        poolStats.stTotalSize        = stats.statistics.blockBytes;
        poolStats.stUsedBytes        = stats.statistics.allocationBytes;

        size_t totalUnusedBytes      = poolStats.stTotalSize - poolStats.stUsedBytes;
        poolStats.stFreeBytes        = totalUnusedBytes; 

        // For VMA this is actually bytes lost to alignment gaps
        poolStats.stOverheadBytes    = totalUnusedBytes - static_cast<size_t>(stats.unusedRangeSizeMax);

        poolStats.stPeakUsed         = std::max(poolStats.stPeakUsed, poolStats.stUsedBytes);

        poolStats.stAllocationCount  = static_cast<size_t>(stats.statistics.allocationCount);
        poolStats.stFreeBlockCount   = static_cast<size_t>(stats.unusedRangeCount);

        if (poolStats.stFreeBlockCount > 0)
        {
            poolStats.stLargestFreeBlock  = stats.unusedRangeSizeMax;
            poolStats.stSmallestFreeBlock = stats.unusedRangeSizeMin;
            poolStats.stAverageFreeBlock  = poolStats.stFreeBytes / poolStats.stFreeBlockCount;
        }
        else
        {
            poolStats.stLargestFreeBlock  = 0;
            poolStats.stSmallestFreeBlock = 0;
            poolStats.stAverageFreeBlock  = 0;
        }

        return poolStats;
    }
}

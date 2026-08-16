#include "pch.h"
#include "graphics/vulkan/VulkanSystemTexture.h"

#include "Engine.h"
#include "graphics/RenderManager.h"
#include "graphics/vulkan/VulkanPoolAllocator.h"

#include <vulkan/vulkan.hpp>

#include "core/stdx.h"
#include "graphics/Helpers.h"
#include "graphics/vulkan/VulkanGraphicsEngine.h"

namespace based
{
    static std_p::map<UUID, vk::Image> s_imageAllocationMap;
    static std::mutex s_imageAllocationMutex;
    
    VulkanSystemTexture* VulkanSystemTexture::AllocateSystemTexture(const TextureBuilder& textureBuilder)
    {
        VulkanSystemTexture* pSysTex = new VulkanSystemTexture();

        pSysTex->m_pImage = pSysTex->FindOrCreateAllocation(textureBuilder);

        vk::ImageAspectFlags nImageAspect;
        if (HasBit(textureBuilder.m_nFlags, eTextureFlags::kDepthOnly))
        {
            nImageAspect |= vk::ImageAspectFlagBits::eDepth;
        }
        if (HasBit(textureBuilder.m_nFlags, eTextureFlags::kStencilOnly))
        {
            nImageAspect |= vk::ImageAspectFlagBits::eStencil;
        }

        if (!HasBit(textureBuilder.m_nFlags, eTextureFlags::kDepthStencil))
        {
            nImageAspect |= vk::ImageAspectFlagBits::eColor;
        }

        auto& GE = dynamic_cast<VulkanGraphicsEngine&>(Engine::Instance().GetGraphicsEngine());
        
        vk::ImageViewCreateInfo viewCI{};
        viewCI
            .setImage(*pSysTex->m_pImage)
            .setViewType(VulkanTextureTypeMap.map(textureBuilder.m_eTextureType))
            .setFormat(VulkanTextureFormatMap.map(textureBuilder.m_eFormat))
            .setSubresourceRange({
                .aspectMask = nImageAspect,
                .levelCount = vk::RemainingMipLevels,
                .layerCount = vk::RemainingArrayLayers
            });
        check(GE.GetDevice().createImageView(&viewCI, GE.GetAllocationCallbacks(), &pSysTex->m_View));

        return pSysTex;
    }

    vk::Image* VulkanSystemTexture::FindOrCreateAllocation(const TextureBuilder& textureBuilder)
    {
        std::scoped_lock lock(s_imageAllocationMutex);
         
        if (s_imageAllocationMap.contains(textureBuilder.m_nUUID))
        {
            return &s_imageAllocationMap[textureBuilder.m_nUUID];
        }
        
        vk::SampleCountFlagBits nSampleCount;
        switch (Engine::Instance().GetRenderManager().GetAntiAliasingMode())
        {
        default:
        case eAntiAliasingMode::kNone:
            nSampleCount = vk::SampleCountFlagBits::e1;
            break;
        case eAntiAliasingMode::kMSAA2x:
            nSampleCount = vk::SampleCountFlagBits::e2;
            break;
        case eAntiAliasingMode::kMSAA4x:
            nSampleCount = vk::SampleCountFlagBits::e4;
            break;
        case eAntiAliasingMode::kMSAA8x:
            nSampleCount = vk::SampleCountFlagBits::e8;
            break;
        }

        vk::ImageUsageFlags nUsage =
            vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
        if (HasBit(textureBuilder.m_nFlags, eTextureFlags::kRenderTarget))
        {
            nUsage |= vk::ImageUsageFlagBits::eColorAttachment;
            nUsage &= ~vk::ImageUsageFlagBits::eTransferDst;
        } else if (HasBit(textureBuilder.m_nFlags, eTextureFlags::kDepthStencil)
                    || HasBit(textureBuilder.m_nFlags, eTextureFlags::kDepthOnly)
                    || HasBit(textureBuilder.m_nFlags, eTextureFlags::kStencilOnly))
        {
            nUsage |= vk::ImageUsageFlagBits::eDepthStencilAttachment;
            nUsage &= ~vk::ImageUsageFlagBits::eTransferDst;
        } else if (HasBit(textureBuilder.m_nFlags, eTextureFlags::kStorage))
        {
            nUsage |= vk::ImageUsageFlagBits::eStorage;
        }

        vk::ImageCreateInfo imageCI{};
        imageCI
            .setImageType(VulkanAltTextureTypeMap.map(textureBuilder.m_eTextureType))
            .setFormat(VulkanTextureFormatMap.map(textureBuilder.m_eFormat))
            .setExtent({
                .width = textureBuilder.m_nWidth,
                .height = textureBuilder.m_nHeight,
                .depth = textureBuilder.m_nDepth})
            .setMipLevels(textureBuilder.m_nNumMips)
            .setArrayLayers(textureBuilder.m_nLayers)
            .setSamples(nSampleCount)
            .setTiling(
                HasBit(textureBuilder.m_nFlags, eTextureFlags::kTiled)
                ? vk::ImageTiling::eOptimal
                : vk::ImageTiling::eLinear)
            .setUsage(nUsage)
            .setInitialLayout(vk::ImageLayout::eUndefined);

        s_imageAllocationMap[textureBuilder.m_nUUID] = nullptr;
        vk::Image* pImage = &s_imageAllocationMap[textureBuilder.m_nUUID];

        auto& GE = dynamic_cast<VulkanGraphicsEngine&>(Engine::Instance().GetGraphicsEngine());
        check(GE.GetDevice().createImage(&imageCI, GE.GetAllocationCallbacks(), pImage));

        vk::MemoryRequirements memRequirements;
        GE.GetDevice().getImageMemoryRequirements(*pImage, &memRequirements);

        m_pBaseAddress = MemoryManager::GraphicsMemAlign(memRequirements.size, memRequirements.alignment);
        BASED_ASSERT(m_pBaseAddress, "Could not allocate graphics memory!");
        if (!m_pBaseAddress) return nullptr;

        VulkanPoolAllocator* pAllocator = dynamic_cast<VulkanPoolAllocator*>(g_pCurrentGraphicsPool->m_pPoolAllocator);
        vk::DeviceMemory gpuBaseAddress = pAllocator->GetBaseGPUAddress();
        vk::DeviceSize gpuBaseOffset    = pAllocator->GetGPUBaseOffset();
        auto maybeHeader = pAllocator->FindHeader(m_pBaseAddress);
        BASED_ASSERT(maybeHeader.has_value(), "");
        if (!maybeHeader.has_value()) return nullptr;

        const VulkanPoolAllocator::Header* pAllocHeader = maybeHeader.value();
        
        VmaVirtualAllocationInfo allocInfo{};
        pAllocator->GetAllocationInfo(pAllocHeader->m_Alloc, &allocInfo);

        vk::DeviceSize allocOffset = allocInfo.offset;

        GE.GetDevice().bindImageMemory(*pImage, gpuBaseAddress, gpuBaseOffset + allocOffset);

        return pImage;
    }
}

#pragma once
#include "graphics/Texture.h"
#include <vulkan/vulkan.hpp>

namespace based
{
    using FilterMap = EnumMap<eTextureFilter, vk::Filter>;
    constexpr FilterMap VulkanFilterMap{
        std::array{
            FilterMap::Mapping{eTextureFilter::kNearest, vk::Filter::eNearest},
            FilterMap::Mapping{eTextureFilter::kLinear, vk::Filter::eLinear}
        }
    };

    using WrapMap = EnumMap<eTextureWrapMode, vk::SamplerAddressMode>;
    constexpr WrapMap VulkanWrapMap{
        std::array{
            WrapMap::Mapping{eTextureWrapMode::kRepeat, vk::SamplerAddressMode::eRepeat},
            WrapMap::Mapping{eTextureWrapMode::kMirroredRepeat, vk::SamplerAddressMode::eMirroredRepeat},
            WrapMap::Mapping{eTextureWrapMode::kClampEdge, vk::SamplerAddressMode::eClampToEdge},
            WrapMap::Mapping{eTextureWrapMode::kClampBorder, vk::SamplerAddressMode::eClampToBorder}
        }
    };

    using TextureTypeMap = EnumMap<eTextureType, vk::ImageViewType>;
    constexpr TextureTypeMap VulkanTextureTypeMap{
        std::array{
            TextureTypeMap::Mapping{eTextureType::k2D, vk::ImageViewType::e2D},
            TextureTypeMap::Mapping{eTextureType::k3D, vk::ImageViewType::e3D},
            TextureTypeMap::Mapping{eTextureType::kCube, vk::ImageViewType::eCube},
            TextureTypeMap::Mapping{eTextureType::k1D, vk::ImageViewType::e1D},
            TextureTypeMap::Mapping{eTextureType::k1DArray, vk::ImageViewType::e1DArray},
            TextureTypeMap::Mapping{eTextureType::k2DArray, vk::ImageViewType::e2DArray},
            TextureTypeMap::Mapping{eTextureType::kCubeArray, vk::ImageViewType::eCubeArray},
        }
    };

    using AltTextureTypeMap = EnumMap<eTextureType, vk::ImageType>;
    constexpr AltTextureTypeMap VulkanAltTextureTypeMap{
        std::array{
            AltTextureTypeMap::Mapping{eTextureType::k2D, vk::ImageType::e2D},
            AltTextureTypeMap::Mapping{eTextureType::k3D, vk::ImageType::e3D},
            AltTextureTypeMap::Mapping{eTextureType::kCube, vk::ImageType::e2D},
            AltTextureTypeMap::Mapping{eTextureType::k1D, vk::ImageType::e1D},
            AltTextureTypeMap::Mapping{eTextureType::k1DArray, vk::ImageType::e2D},
            AltTextureTypeMap::Mapping{eTextureType::k2DArray, vk::ImageType::e2D},
            AltTextureTypeMap::Mapping{eTextureType::kCubeArray, vk::ImageType::e2D},
        }
    };

    using TextureFormatMap = EnumMap<eTextureFormat, vk::Format>;
    constexpr TextureFormatMap VulkanTextureFormatMap{
        std::array{
            // Color
            TextureFormatMap::Mapping{eTextureFormat::kRGBA8_UNORM,     vk::Format::eR8G8B8A8Unorm},
            TextureFormatMap::Mapping{eTextureFormat::kRGBA8_SRGB,      vk::Format::eR8G8B8A8Srgb},
            TextureFormatMap::Mapping{eTextureFormat::kBGRA8_UNORM,     vk::Format::eB8G8R8A8Unorm},
            TextureFormatMap::Mapping{eTextureFormat::kBGRA8_SRGB,      vk::Format::eB8G8R8A8Srgb},
            TextureFormatMap::Mapping{eTextureFormat::kRGBA16_SFLOAT,   vk::Format::eR16G16B16A16Sfloat},
            TextureFormatMap::Mapping{eTextureFormat::kRGBA32_SFLOAT,   vk::Format::eR32G32B32A32Sfloat},

            // Depth
            TextureFormatMap::Mapping{eTextureFormat::kD32_SFLOAT,          vk::Format::eD32Sfloat},
            TextureFormatMap::Mapping{eTextureFormat::kD24_UNORM_S8_UINT,   vk::Format::eD24UnormS8Uint},
            TextureFormatMap::Mapping{eTextureFormat::kD32_SFLOAT_S8_UINT,  vk::Format::eD32SfloatS8Uint},
            TextureFormatMap::Mapping{eTextureFormat::kD16_UNORM,           vk::Format::eD16Unorm},

            // Compressed
            TextureFormatMap::Mapping{eTextureFormat::kBC7_UNORM,        vk::Format::eBc7UnormBlock},
            TextureFormatMap::Mapping{eTextureFormat::kBC7_SRGB,         vk::Format::eBc7SrgbBlock},
            TextureFormatMap::Mapping{eTextureFormat::kBC5_UNORM,              vk::Format::eBc5UnormBlock},
            TextureFormatMap::Mapping{eTextureFormat::kBC4_UNORM,              vk::Format::eBc4UnormBlock},
            TextureFormatMap::Mapping{eTextureFormat::kBC1_RGB_UNORM,              vk::Format::eBc1RgbUnormBlock},
            TextureFormatMap::Mapping{eTextureFormat::kASTC_4x4_UNORM,   vk::Format::eAstc4x4UnormBlock},
            TextureFormatMap::Mapping{eTextureFormat::kASTC_4x4_SRGB,    vk::Format::eAstc4x4SrgbBlock},
            TextureFormatMap::Mapping{eTextureFormat::kASTC_6x6_UNORM,   vk::Format::eAstc6x6UnormBlock},
            TextureFormatMap::Mapping{eTextureFormat::kASTC_6x6_SRGB,    vk::Format::eAstc6x6SrgbBlock},
            TextureFormatMap::Mapping{eTextureFormat::kASTC_8x8_UNORM,   vk::Format::eAstc8x8UnormBlock},
            TextureFormatMap::Mapping{eTextureFormat::kASTC_8x8_SRGB,    vk::Format::eAstc8x8SrgbBlock},

            // Others
            TextureFormatMap::Mapping{eTextureFormat::kRGB32_SFLOAT,     vk::Format::eR32G32B32Sfloat},
            TextureFormatMap::Mapping{eTextureFormat::kRG32_SFLOAT,      vk::Format::eR32G32Sfloat},
        }
    };
    
    class VulkanSystemTexture final : public NonCopyable
    {
        friend class ResourceManager;
    public:

    private:
        VulkanSystemTexture() = default;
        ~VulkanSystemTexture() = default;

        static VulkanSystemTexture* AllocateSystemTexture(const TextureBuilder& textureBuilder);
        
        vk::Image* FindOrCreateAllocation(const TextureBuilder& textureBuilder);

        // Multiple textures can reference the same image
        vk::Image* m_pImage = nullptr;
        vk::ImageView m_View = nullptr;
        // The image's actual address for CPU-visible memory. For device-local it's only used to get the VMA alloc info.
        void* m_pBaseAddress = nullptr;
    };
}

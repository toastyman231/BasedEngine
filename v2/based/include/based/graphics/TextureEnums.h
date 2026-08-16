#pragma once
#include "BasedDefines.h"

namespace based
{
    enum class eTextureFilter : uint8
    {
        kNearest,
        kLinear,
        kCount
    };

    enum class eTextureWrapMode : uint8
    {
        kRepeat,
        kMirroredRepeat,
        kClampEdge,
        kClampBorder,
        kCount
    };

    enum class eTextureType : uint8
    {
        k2D,
        k3D,
        kCube,
        k1D, // This doesn't match Vulkan because it isn't first, but I wanted 2D to be the default
        k1DArray,
        k2DArray,
        kCubeArray,
        kCount
    };

    // A selection from vk::Format, no need to support literally hundreds (for now)
    enum class eTextureFormat : uint8
    {
        kNone,
        
        // Color
        kRGBA8_UNORM,
        kRGBA8_SRGB,
        kBGRA8_UNORM,
        kBGRA8_SRGB,
        kRGBA16_SFLOAT,
        kRGBA32_SFLOAT,

        // Depth (ordered by preference)
        kD32_SFLOAT_S8_UINT,
        kD24_UNORM_S8_UINT,
        kD32_SFLOAT,
        kD16_UNORM,

        // Compressed
        kBC7_UNORM,
        kBC7_SRGB,
        kBC5_UNORM,
        kBC4_UNORM,
        kBC1_RGB_UNORM,
        kASTC_4x4_UNORM,
        kASTC_4x4_SRGB,
        kASTC_6x6_UNORM,
        kASTC_6x6_SRGB,
        kASTC_8x8_UNORM,
        kASTC_8x8_SRGB,

        // Others
        kRGB32_SFLOAT,
        kRG32_SFLOAT,

        kCount
    };
    
    enum class eTextureFlags : uint32
    {
        kNone = 0,
        // Usage
        kRenderTarget   = 1u << 0,
        kDepthOnly      = 1u << 1,
        kStencilOnly    = 1u << 2,
        kDepthStencil   = kDepthOnly | kStencilOnly,
        kStorage        = 1u << 2,
        // Format
        kTiled          = 1u << 3,
        kCompressed     = 1u << 4,
    };
    DEFINE_ENUM_CLASS_BITWISE_OPERATORS(eTextureFlags);
}

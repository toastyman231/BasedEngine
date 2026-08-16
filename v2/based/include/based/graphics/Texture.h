#pragma once
#include "core/UUID.h"
#include "memory/PlatformMemUtils.h"
#include "SystemTexture.h"

namespace based
{
    constexpr size_t kMaxTextureNameLength = 160;

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

    class TextureBuilder final
    {
        friend class ResourceManager;
        friend class VulkanSystemTexture;
    public:

        TextureBuilder& SetName(const char* pStrName)
        {
            strncpy(m_strName, pStrName, strlen(pStrName));
            return *this;
        }

        TextureBuilder& SetSize(uint32 nWidth, uint32 nHeight)
        {
            m_nWidth = nWidth;
            m_nHeight = nHeight;
            return *this;
        }

        TextureBuilder& SetDepth(uint32 nDepth)
        {
            m_nDepth = nDepth;
            return *this;
        }

        TextureBuilder& SetLayers(uint32 nLayers)
        {
            m_nLayers = nLayers;
            return *this;
        }

        TextureBuilder& SetNumMips(uint32 nNumMips)
        {
            m_nNumMips = nNumMips;
            return *this;
        }

        TextureBuilder& SetType(eTextureType eType)
        {
            m_eTextureType = eType;
            return *this;
        }

        TextureBuilder& SetFormat(eTextureFormat eFormat)
        {
            m_eFormat = eFormat;
            return *this;
        }

        TextureBuilder& SetFlags(eTextureFlags nFlags)
        {
            m_nFlags |= nFlags;
            return *this;
        }

        TextureBuilder& SetPoolToUse(ePoolIdentifier ePoolToUse)
        {
            m_ePoolToUse = ePoolToUse;
            return *this;
        }

        char m_strName[kMaxTextureNameLength] = "Unnamed Texture";
        uint32 m_nWidth = 0, m_nHeight = 0, m_nDepth = 1, m_nLayers = 1, m_nNumMips = 1;
        eTextureFlags m_nFlags = eTextureFlags::kNone;
        eTextureType m_eTextureType = eTextureType::k2D;
        eTextureFormat m_eFormat = eTextureFormat::kRGBA8_UNORM;
        ePoolIdentifier m_ePoolToUse = ePoolIdentifier::kTextureGPUPool;

    private:
        // The actual Texture will set this after creation, so we can continue to use this object
        // for stuff like looking up existing allocations without passing both the builder and ID around,
        // but users won't be able to specify a specific UUID
        UUID m_nUUID;
    };

    class Texture final : public NonCopyable
    {
        friend class ResourceManager;
    public:
        static constexpr bool IsResource = true;

        std::string_view GetName()        const { return m_strName; }
        UUID             GetUUID()        const { return m_nResourceID; }
        uint32           GetWidth()       const { return m_nWidth; }
        uint32           GetHeight()      const { return m_nHeight; }
        Vec2             GetSize()        const { return {m_nWidth, m_nHeight}; }
        eTextureType     GetTextureType() const { return m_eTextureType; }

        bool IsRenderTarget() const { return HasBit(m_nFlags, eTextureFlags::kRenderTarget); }
        bool IsDepthStencil() const { return HasBit(m_nFlags, eTextureFlags::kDepthStencil); }
        bool IsTiled()        const { return HasBit(m_nFlags, eTextureFlags::kTiled); }

    private:
        Texture() = default;
        ~Texture() = default;

        char m_strName[kMaxTextureNameLength];
        UUID m_nResourceID;
        SystemTexture* m_pSystemTexture; 
        uint32 m_nWidth, m_nHeight, m_nDepth, m_nLayers, m_nNumMips;
        eTextureFlags m_nFlags;
        eTextureFormat m_eFormat;
        eTextureType m_eTextureType;
        
    };
}

#pragma once
#include "core/UUID.h"
#include "memory/PlatformMemUtils.h"
#include "SystemTexture.h"
#include "TextureEnums.h"

namespace based
{
    constexpr size_t kMaxTextureNameLength = 160;

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

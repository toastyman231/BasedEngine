#include "pch.h"
#include "memory/ResourceManager.h"

#include "graphics/SystemTexture.h"
#include "graphics/Texture.h"
#include "memory/MemoryPoolHeader.h"

namespace based
{
    template <typename T> requires Resource<T>
    ResourceTable<T>::ResourceTable()
        : m_vResourceSlots(ResourceTraits<T>::kInitialSlots)
    {
    }

    template <typename T> requires Resource<T>
    Handle<T> ResourceTable<T>::InsertResource(T* pResource)
    {
        BASED_ASSERT(pResource, "Don't insert null resources into the table!");

        std::scoped_lock lock(m_mutex);

        size_t nSlotIndex = m_stFirstAvailableSlot;
        uint32 nGeneration;
        {
            // Make sure slot isn't valid after this because we may re-allocate the array
            
            ResourceSlot<T>& slot = m_vResourceSlots[nSlotIndex];

            slot.m_pResource = pResource;
            slot.m_nReferences = 1; // Start at 1 when we insert, creating new Handles will increment this
            slot.m_bOccupied = true;
            // Do not increment the generation here! We do that when we free the slot.

            nGeneration = slot.m_nGeneration;
        }

        bool bFoundNextSlot = false;
        for (size_t i = nSlotIndex + 1; i < m_vResourceSlots.size(); ++i)
        {
            if (!m_vResourceSlots[i].m_bOccupied)
            {
                m_stFirstAvailableSlot = i;
                bFoundNextSlot = true;
                break;
            }
        }

        if (!bFoundNextSlot)
        {
            size_t nOldSize = m_vResourceSlots.size();
            size_t nNewSize = std::max(static_cast<size_t>(static_cast<double>(nOldSize) * 1.5), nOldSize + 1);
            
            m_vResourceSlots.resize(nNewSize);
            m_stFirstAvailableSlot = nOldSize;
        }

        return Handle<T>(static_cast<uint32>(nSlotIndex), nGeneration);
    }

    template <typename T> requires Resource<T>
    void ResourceTable<T>::RemoveResource(Handle<T> hResource)
    {
        BASED_ASSERT(hResource != Handle<T>::Null(), "Do not RemoveResource on a null handle!");
        if (hResource == Handle<T>::Null()) return;

        std::scoped_lock lock(m_mutex);

        ResourceSlot<T>& slot = m_vResourceSlots[hResource.m_nIndex];
        ++slot.m_nGeneration;
        slot.m_nReferences = 0;
        slot.m_bOccupied = false;
        slot.m_pResource = nullptr;

        if (hResource.m_nIndex < m_stFirstAvailableSlot) m_stFirstAvailableSlot = hResource.m_nIndex;
    }

    Handle<Texture> ResourceManager::CreateTexture(TextureBuilder* pBuilder)
    {
        BASED_ASSERT(pBuilder, "Can't create a texture from a null builder!");
        if (!pBuilder) return Handle<Texture>::Null();

        AllocatorScope ac(ePoolIdentifier::kTextureCPUPool, pBuilder->m_ePoolToUse);

        Texture* pTex = new Texture();
        strncpy(pTex->m_strName, pBuilder->m_strName, strlen(pBuilder->m_strName));

        // Correct depth based on texture type
        switch (pBuilder->m_eTextureType)
        {
        case eTextureType::k1D:
        case eTextureType::k2D:
        case eTextureType::kCube:
        case eTextureType::k1DArray:
        case eTextureType::k2DArray:
        case eTextureType::kCubeArray:
            pBuilder->m_nDepth = 1;
            break;
        case eTextureType::k3D:
            break;
        case eTextureType::kCount:
            BASED_ASSERT(false, "You shouldn't be passing kCount in as a value!");
            break;
        }

        // Ensure layers are correct as well
        switch (pBuilder->m_eTextureType)
        {
        case eTextureType::k1D:
        case eTextureType::k2D:
        case eTextureType::k3D:
            pBuilder->m_nLayers = 1;
            break;
        case eTextureType::k1DArray:
        case eTextureType::k2DArray:
            break;
        case eTextureType::kCubeArray:
            pBuilder->m_nLayers = pBuilder->m_nLayers * 6;
            break;
        case eTextureType::kCube:
            pBuilder->m_nLayers = 6;
            break;
        case eTextureType::kCount:
            BASED_ASSERT(false, "You shouldn't be passing kCount in as a value!");
            break;
        }

        pTex->m_nWidth = pBuilder->m_nWidth;
        pTex->m_nHeight = pBuilder->m_nHeight;
        pTex->m_nDepth = pBuilder->m_nDepth;
        pTex->m_nLayers = pBuilder->m_nLayers;
        pTex->m_nNumMips = pBuilder->m_nNumMips;
        pTex->m_nFlags = pBuilder->m_nFlags;
        pTex->m_eTextureType = pBuilder->m_eTextureType;
        pTex->m_eFormat = pBuilder->m_eFormat;

        // So the system texture can look up an existing allocation (for aliases, etc.)
        pBuilder->m_nUUID = pTex->m_nResourceID;

        pTex->m_pSystemTexture = SystemTexture::AllocateSystemTexture(*pBuilder);

        return GetResourceTableFor<Texture>().InsertResource(pTex);
    }

    Handle<Texture> ResourceManager::FindTextureByUUID(UUID nUUID)
    {
        std::scoped_lock lock(m_tablesMutex);
        
        auto& slots = GetResourceTableFor<Texture>().m_vResourceSlots;
        auto it = std::ranges::find_if(slots,
            [nUUID](const ResourceSlot<Texture>& slot)
            {
                return slot.m_pResource && slot.m_pResource->GetUUID() == nUUID;
            });
        if (it != slots.end())
        {
            return Handle<Texture>(std::distance(slots.begin(), it), it->m_nGeneration);
        }

        return Handle<Texture>::Null();
    }

    ResourceManager* ResourceManager::Create()
    {
        static bool bDoOnce = false;
        BASED_ASSERT(!bDoOnce, "Trying to create a second RenderManager is not allowed! Get it from the Engine!");

        AllocatorScope ac(ePoolIdentifier::kPersistentPool);
        ResourceManager* pResourceManager = new ResourceManager();
        BASED_ASSERT(pResourceManager, "Something went wrong trying to create ResourceManager!");
        if (pResourceManager)
        {
            bDoOnce = true;
            return pResourceManager;
        }

        return nullptr;
    }

    void ResourceManager::Shutdown()
    {
        // Nothing to do here for now
    }
}

#pragma once
#include "Engine.h"
#include "memory/ResourceManager.h"

namespace based
{
    template <typename T>
    struct ResourceSlot;
    
    template <typename ResourceType>
    class Handle final : public ExplicitlyCopyable
    {
        friend class ResourceManager;
    public:
        ResourceType* operator ->() noexcept
        {
            return Get();
        }

        bool IsValid() const noexcept
        {
            bool bNotNull = this != Null();
            bool bValidIndex = m_nIndex != kInvalidIndex;
            bool bOccupied = false;
            bool bValidGeneration = false;
            bool bValidPointer = false;

            if (bNotNull && bValidIndex)
            {
                ResourceSlot<ResourceType>& slot = Engine::Instance().GetResourceManager()
                                                    .GetResourceTableFor<ResourceType>()
                                                    .m_vResourceSlots[m_nIndex];
                bOccupied = slot.m_bOccupied;
                bValidGeneration = slot.m_nGeneration == m_nGeneration;
                bValidPointer = slot.m_pResource;
            }
            
            return bNotNull && bValidIndex && bOccupied && bValidGeneration && bValidPointer;
        }

        constexpr explicit operator bool() const noexcept
        {
            return IsValid();
        }

        ResourceType* Get()
        {
            if (!IsValid()) return nullptr;
            return Engine::Instance().GetResourceManager()
                        .GetResourceTableFor<ResourceType>()
                        .m_vResourceSlots[m_nIndex].m_pResource;
        }

        static constexpr Handle Null() noexcept { return {}; }

    private:
        static constexpr uint32 kInvalidIndex = std::numeric_limits<uint32>::max();

        Handle(uint32 nIndex, uint32 nGeneration)
            : m_nIndex(nIndex), m_nGeneration(nGeneration)
        {
            AddRef();
        }

        ~Handle()
        {
            UnRef();
        }

        void AddRef()
        {
            ++Engine::Instance().GetResourceManager()
                                .GetResourceTableFor<ResourceType>()
                                    .m_vResourceSlots[m_nIndex].nReferences;
        }

        void UnRef()
        {
            --Engine::Instance().GetResourceManager()
                                .GetResourceTableFor<ResourceType>()
                                    .m_vResourceSlots[m_nIndex].nReferences;
        }
        
        uint32 m_nIndex = kInvalidIndex;
        uint32 m_nGeneration = 0;
    };
}

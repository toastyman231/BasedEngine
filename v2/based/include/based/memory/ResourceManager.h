#pragma once
#include "BasedTypeTraits.h"
#include "core/Handle.h"
#include "core/stdx.h"

namespace based
{
    class Texture;
    class TextureBuilder;

    template <typename T> requires Resource<T>
    struct ResourceSlot final
    {
        T* m_pResource;
        uint32 m_nReferences; // We reference count the resource, but this is just for tracking, not memory management
        uint32 m_nGeneration; // Does this slot still point to the same resource as some handle?
        bool m_bOccupied;     // Does this slot have anything in it at all?
    };

    // Just so we can track all resource tables in one list
    class IResourceTable : public NonCopyable
    {
    public:
        virtual ~IResourceTable() = default;
    };
    
    template <typename T> requires Resource<T>
    class ResourceTable final : public IResourceTable
    {
        friend class ResourceManager;

        ResourceTable();

        Handle<T> InsertResource(T* pResource);
        void RemoveResource(Handle<T> hResource);
        
        std_p::vector<ResourceSlot<T>> m_vResourceSlots;
        size_t m_stFirstAvailableSlot = 0;
        mutable std::shared_mutex m_mutex;
    };
    
    class ResourceManager final : public NonMoveable
    {
        friend class Engine;
        template <typename T>
        friend class Handle;
    public:

        Handle<Texture> CreateTexture(TextureBuilder* pBuilder);

        Handle<Texture> FindTextureByUUID(UUID nUUID);

    private:
        ResourceManager() = default;
        ~ResourceManager() = default;

        static ResourceManager* Create();

        void Shutdown();
        
        static size_t GetNextTypeID()
        {
            static std::atomic<size_t> nCounter = 0;
            return nCounter.fetch_add(1, std::memory_order_relaxed);
        }

        template <typename T> requires Resource<T>
        ResourceTable<T>& GetResourceTableFor()
        {
            // ID is calculated once per type T, so we don't have to pay the cost of type erasure
            static const size_t stTypeID = GetNextTypeID();
            AllocatorScope ac(ePoolIdentifier::kPersistentPool); // Make sure the ResourceTable itself goes into the persistent pool

            {
                std::shared_lock readLock(m_tablesMutex);
                if (stTypeID < m_vResourceTables.size() && m_vResourceTables[stTypeID])
                {
                    return static_cast<ResourceTable<T>&>(*m_vResourceTables[stTypeID]);
                }
            }

            std::scoped_lock writeLock(m_tablesMutex);
            if (stTypeID >= m_vResourceTables.size())
            {
                m_vResourceTables.resize(stTypeID + 1);
            }
            if (!m_vResourceTables[stTypeID])
            {
                m_vResourceTables[stTypeID] = std::make_unique<ResourceTable<T>>();
            }

            return static_cast<ResourceTable<T>&>(*m_vResourceTables[stTypeID]);
        }

        std_p::vector<std::unique_ptr<IResourceTable>> m_vResourceTables;
        mutable std::shared_mutex m_tablesMutex;
    };
}

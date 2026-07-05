#include "based/main.h"

namespace based
{

    class Sandbox final : public App
    {
    public:
        void Initialize() override
        {
            BASED_INFO("=======================   INITIALIZING SANDBOX    =======================");
            BASED_INFO("======================= DONE INITIALIZING SANDBOX =======================");
        }
        
        void Update(float fDeltaTime) override
        {
        }
        
        void Render() override
        {
        }
        
        void Shutdown() override
        {
            BASED_INFO("======================= SHUTTING DOWN SANDBOX =======================");
        }
    };

    App* CreateApp()
    {
        return new Sandbox();
    }

    // Don't actually need all that much memory for testing right now.
    const EngineMemoryPoolDescriptorList& GetMemoryPoolDescriptors()
    {
        static constexpr EngineMemoryPoolDescriptorList DefaultPoolDescriptors = {
            .pools = {{
                { 
                    ePoolIdentifier::kPersistentPool, 
                    PoolDescriptor
                    {
                        .m_strPoolName = "Persistent",
                        .m_stPoolSize = gib_to_bytes(3),
                        .m_ePoolID = to_underlying(ePoolIdentifier::kPersistentPool),
                        .m_eParentPoolID = to_underlying(ePoolIdentifier::kInvalid),
                        .m_bIsGPUPool = false
                    }
                },
                { 
                    ePoolIdentifier::kScratchCPUPool, 
                    PoolDescriptor
                    {
                        .m_strPoolName = "Scratch",
                        .m_stPoolSize = gib_to_bytes(1),
                        .m_ePoolID = to_underlying(ePoolIdentifier::kScratchCPUPool),
                        .m_eParentPoolID = to_underlying(ePoolIdentifier::kPersistentPool),
                        .m_bIsGPUPool = false
                    }
                }
            }}
        };

        return DefaultPoolDescriptors;
    }
}

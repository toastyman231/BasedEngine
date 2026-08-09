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

        WindowProperties GetWindowProperties() override
        {
            WindowProperties props;
            props.title = "Sandbox";
            props.w = 1280;
            props.h = 720;

            return props;
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
                        .m_eGPUMemRequirements = eGPUMemoryRequirements::kNone,
                        .m_ePoolUsageIntent = ePoolUsageIntent::kNone,
                        .m_bIsGPUPool = false
                    }
                },
                { 
                    ePoolIdentifier::kPersistentGraphicsPool, 
                    PoolDescriptor
                    {
                        .m_strPoolName = "Persistent Graphics",
                        .m_stPoolSize = gib_to_bytes(1),
                        .m_ePoolID = to_underlying(ePoolIdentifier::kPersistentGraphicsPool),
                        .m_eParentPoolID = to_underlying(ePoolIdentifier::kInvalid),
                        .m_eGPUMemRequirements = eGPUMemoryRequirements::kCPUBARPreferred,
                        .m_ePoolUsageIntent = ePoolUsageIntent::kBuffers,
                        .m_bIsGPUPool = true
                    }
                },
                { 
                    ePoolIdentifier::kPersistentDeviceGraphicsPool, 
                    PoolDescriptor
                    {
                        .m_strPoolName = "Persistent Device Graphics",
                        .m_stPoolSize = gib_to_bytes(1),
                        .m_ePoolID = to_underlying(ePoolIdentifier::kPersistentDeviceGraphicsPool),
                        .m_eParentPoolID = to_underlying(ePoolIdentifier::kInvalid),
                        .m_eGPUMemRequirements = eGPUMemoryRequirements::kGPUBARPreferred,
                        .m_ePoolUsageIntent = ePoolUsageIntent::kNone,
                        .m_bIsGPUPool = true
                    }
                },
                { 
                    ePoolIdentifier::kTextureCPUPool, 
                    PoolDescriptor
                    {
                        .m_strPoolName = "Texture CPU",
                        .m_stPoolSize = mib_to_bytes(50),
                        .m_ePoolID = to_underlying(ePoolIdentifier::kTextureCPUPool),
                        .m_eParentPoolID = to_underlying(ePoolIdentifier::kPersistentPool),
                        .m_eGPUMemRequirements = eGPUMemoryRequirements::kNone,
                        .m_ePoolUsageIntent = ePoolUsageIntent::kNone,
                        .m_bIsGPUPool = false
                    }
                },
                { 
                    ePoolIdentifier::kTextureGPUPool, 
                    PoolDescriptor
                    {
                        .m_strPoolName = "Texture GPU",
                        .m_stPoolSize = gib_to_bytes(4),
                        .m_ePoolID = to_underlying(ePoolIdentifier::kTextureGPUPool),
                        .m_eParentPoolID = to_underlying(ePoolIdentifier::kInvalid),
                        .m_eGPUMemRequirements = eGPUMemoryRequirements::kGPUOnly,
                        .m_ePoolUsageIntent = ePoolUsageIntent::kNone,
                        .m_bIsGPUPool = true
                    }
                },
                { 
                    ePoolIdentifier::kTextureSmallGPUPool, 
                    PoolDescriptor
                    {
                        .m_strPoolName = "Small Texture GPU",
                        .m_stPoolSize = mib_to_bytes(80),
                        .m_ePoolID = to_underlying(ePoolIdentifier::kTextureSmallGPUPool),
                        .m_eParentPoolID = to_underlying(ePoolIdentifier::kInvalid),
                        .m_eGPUMemRequirements = eGPUMemoryRequirements::kGPUOnly,
                        .m_ePoolUsageIntent = ePoolUsageIntent::kNone,
                        .m_bIsGPUPool = true
                    }
                },
                { 
                    ePoolIdentifier::kScratchCPUPool, 
                    PoolDescriptor
                    {
                        .m_strPoolName = "Scratch CPU",
                        .m_stPoolSize = gib_to_bytes(1),
                        .m_ePoolID = to_underlying(ePoolIdentifier::kScratchCPUPool),
                        .m_eParentPoolID = to_underlying(ePoolIdentifier::kPersistentPool),
                        .m_eGPUMemRequirements = eGPUMemoryRequirements::kNone,
                        .m_ePoolUsageIntent = ePoolUsageIntent::kScratch,
                        .m_bIsGPUPool = false
                    }
                },
                { 
                    ePoolIdentifier::kScratchGPUPool, 
                    PoolDescriptor
                    {
                        .m_strPoolName = "Scratch GPU",
                        .m_stPoolSize = gib_to_bytes(3),
                        .m_ePoolID = to_underlying(ePoolIdentifier::kScratchGPUPool),
                        .m_eParentPoolID = to_underlying(ePoolIdentifier::kInvalid),
                        .m_eGPUMemRequirements = eGPUMemoryRequirements::kCPUBARPreferred,
                        .m_ePoolUsageIntent = ePoolUsageIntent::kScratch | ePoolUsageIntent::kBuffers,
                        .m_bIsGPUPool = true
                    }
                },
                { 
                    ePoolIdentifier::kStagingPool, 
                    PoolDescriptor
                    {
                        .m_strPoolName = "Staging",
                        .m_stPoolSize = gib_to_bytes(1),
                        .m_ePoolID = to_underlying(ePoolIdentifier::kStagingPool),
                        .m_eParentPoolID = to_underlying(ePoolIdentifier::kInvalid),
                        .m_eGPUMemRequirements = eGPUMemoryRequirements::kCPUOnly,
                        .m_ePoolUsageIntent = ePoolUsageIntent::kScratch | ePoolUsageIntent::kBuffers,
                        .m_bIsGPUPool = true
                    }
                },
            }}
        };
        
        return DefaultPoolDescriptors;
    }
}

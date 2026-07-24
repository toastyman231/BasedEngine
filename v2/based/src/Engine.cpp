#include "pch.h"
#include "Engine.h"

#include "core/NewDelete.h"
#define FLAG_IMPLEMENTATION
#define FLAG_ALLOC my_malloc
#define FLAG_REALLOC my_realloc
#define FLAG_FREE my_free
#include "VERSION.h"
#include "core/App.h"
#include "core/BasedTime.h"
#include "core/Window.h"
#include "graphics/RenderManager.h"
#include "tsoding/flag.h"

namespace based
{
    Engine& Engine::Instance()
    {
        static Engine instance;
        return instance;
    }
    
    const Engine& Engine::ConstInstance()
    {
        return Instance();
    }
    
    void Engine::Run(App* app)
    {
        BASED_ASSERT(!m_pApp, "Trying to call Engine::Run when a valid app already exists!");
        if (m_pApp) return;

        m_pApp = app;
        if (Initialize())
        {
#ifdef __EMSCRIPTEN__
            auto callback = [](void* arg)
            {
                auto pEngine = static_cast<Engine*>(arg);
                pEngine->Run_Internal();
                if (!pEngine->m_bIsRunning) pEngine->Shutdown();
            };
            emscripten_set_main_loop_arg(callback, this, 0, true);
#else
            while (m_bIsRunning)
            {
                Run_Internal();
            }

            Shutdown();
#endif
        } else
        {
            BASED_ASSERT(false, "A fatal error occurred during engine initialization! Check previous logs for more info.");
            Shutdown();
        }
    }

    void Engine::Run_Internal()
    {
        Time::UpdateUnscaledTime();
        const float unscaledTimeDelta = Time::GetUnscaledTime() - Time::mUnscaledLastFrameTime;
        Time::SetUnscaledDelta(unscaledTimeDelta);
        Time::UpdateTime();
        const float timeDelta = Time::GetTime() - Time::mLastFrameTime;
        Time::SetDelta(timeDelta);

        Update(timeDelta);
        Render();
    }

    // Named to be consistent with the other functions in flag.h
    void* Engine::flag_c_get(void* c, const char* name)
    {
        Flag_Context *fc = static_cast<Flag_Context*>(c);

        for (size_t i = 0; i < fc->flags_count; ++i) {
            Flag *f = &fc->flags[i];
            if (strcmp(f->name, name) == 0) {
                return flag__get_ref(f);
            }
        }

        return nullptr;
    }

    void* Engine::GetGlobalFlagContext()
    {
        return &flag_global_context;
    }

    IGraphicsEngine& Engine::GetGraphicsEngine() const
    {
        BASED_ASSERT(m_pRenderManager, "Trying to get render manager when one doesn't exist!");
        return m_pRenderManager->GetGraphicsEngine();
    }

    void Engine::GetInfo()
    {
        BASED_INFO("BasedEngine v{}.{}.{}", ENGINE_VERSION_MAJOR, ENGINE_VERSION_MINOR, ENGINE_VERSION_PATCH);
#ifdef BASED_CONFIG_DEVELOPMENT
        BASED_DEBUG("Configuration: DEVELOPMENT");
#elif defined(BASED_CONFIG_DEBUG)
        BASED_DEBUG("Configuration: DEBUG");
#elif defined(BASED_CONFIG_RELEASE)
        BASED_DEBUG("Configuration: RELEASE");
#endif
        BASED_WARN("Platform: {}", PlatformEngine::GetPlatformName());
    }
    
    bool Engine::Initialize()
    {
        BASED_ASSERT(!m_bIsInitialized, "Attempting to call Engine::Initialize() more than once!");
        if (m_bIsInitialized) return false;

        bool bSuccess = false;
        GetInfo();

        DeclareAndParseFlags();

        WindowProperties props = m_pApp->GetWindowProperties();
        m_pWindow = WindowFactory::Create(props);

        // Initialize managers
        m_pRenderManager = RenderManager::Create();
        m_pRenderManager->Initialize();
        
        m_pApp->Initialize();
        bSuccess = true; // For now

        m_bIsInitialized = true;
        m_bIsRunning = true;
        return bSuccess;
    }
    
    bool Engine::DeclareAndParseFlags()
    {
        // Declare engine flags here, before user app flags
        if (m_pApp) m_pApp->DeclareFlags();

        if (!flag_parse(argc, argv))
        {
            if (m_pApp) m_pApp->PrintFlagUsage();
            flag_print_error(stderr);
            return false;
        }

        argc = flag_rest_argc();
        argv = flag_rest_argv();
        return true;
    }
    
    void Engine::Update(float fDeltaTime)
    {
        m_pWindow->PumpEvents();
        if (m_pApp) m_pApp->Update(fDeltaTime);
    }
    
    void Engine::Render()
    {
        if (m_pApp) m_pApp->Render();
    }

    void Engine::Shutdown()
    {
        m_bIsRunning = false;
        m_bIsInitialized = false;

        if (m_pApp) m_pApp->Shutdown();

        // Shutdown managers in reverse order
        m_pRenderManager->Shutdown();

        if (m_pWindow) m_pWindow->Shutdown();

        LogManager::Shutdown();
    }
}

#pragma once

namespace based {
    class ResourceManager;
    class RenderManager;
    class IWindow;
    class IGraphicsEngine;
}

namespace based
{
    class App;
    class Engine final
    {
    public:
        static Engine& Instance();
        static const Engine& ConstInstance();

        App& GetApp() const
        {
            BASED_ASSERT(m_pApp, "Trying to access the app before it's been created!");
            return *m_pApp;
        }

        const App& GetConstApp() const
        {
            return GetApp();
        }

        IWindow* GetWindow() const
        {
            return m_pWindow;
        }

        void Run(App* app);
        void Quit() { m_bIsRunning = false; }

        void SetArgs(int argc_in, char** argv_in)
        {
            argc = argc_in;
            argv = argv_in;
        }

        template <typename T>
        static T GetFlag(const char* name, T fallback)
        {
            T* v = static_cast<T*>(flag_c_get(GetGlobalFlagContext(), name));
            return v ? *v : fallback;
        }

        RenderManager& GetRenderManager() const
        {
            BASED_ASSERT(m_pRenderManager, "Trying to get render manager when one doesn't exist!");
            return *m_pRenderManager;
        }

        ResourceManager& GetResourceManager() const
        {
            BASED_ASSERT(m_pResourceManager, "Trying to get resource manager when one doesn't exist!");
            return *m_pResourceManager;
        }

        IGraphicsEngine& GetGraphicsEngine() const;
        
    private:
        Engine() = default;
        ~Engine() = default;

        void GetInfo();
        bool DeclareAndParseFlags();

        void Run_Internal();

        [[nodiscard]] bool Initialize();
        void Shutdown();
        void Update(float fDeltaTime);
        void Render();

        // Flag helpers
        static void* flag_c_get(void *c, const char *name);
        static void* GetGlobalFlagContext();

        App* m_pApp = nullptr;
        IWindow* m_pWindow = nullptr;
        int argc;
        char** argv;

        // Managers
        RenderManager* m_pRenderManager;
        ResourceManager* m_pResourceManager;

        bool m_bIsRunning = false;
        bool m_bIsInitialized = false;
    };
}

#ifdef BASED_PLATFORM_WINDOWS
#include "platform/windows/WindowsPlatformEngine.h"
#elif defined(BASED_PLATFORM_WEB)
#include "platform/emscripten/WebPlatformEngine.h"
#else
#error "You need to implement PlatformEngine for this platform!"
#endif
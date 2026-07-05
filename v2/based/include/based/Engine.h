#pragma once

namespace based
{
    class App;
    class Engine final
    {
    public:
        static Engine& Instance();
        static const Engine& ConstInstance();

        void Run(App* app);
        void Quit() { m_bIsRunning = true; }

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
        int argc;
        char** argv;

        bool m_bIsRunning = false;
        bool m_bIsInitialized = false;
    };
}

#ifdef BASED_PLATFORM_WINDOWS
#include "platform/windows/WindowsPlatformEngine.h"
#else
#error "You need to implement PlatformEngine for this platform!"
#endif
#pragma once

#include "core/window.h"
#include "managers/logmanager.h"
#include "managers/rendermanager.h"
#include "based/managers/uimanager.h"
#include "memory/arena.h"

namespace based
{
	class App;
    class Engine
    {
    public:
        static Engine& Instance();
        ~Engine() = default;

        void Run(App* app);
        static void SetIcon(std::string path);
        inline void Quit() { mIsRunning = false; }

        inline App& GetApp() const { return *mApp; }
        inline core::Window& GetWindow() { return mWindow; }
        static inline Engine* GetRawEngineInstance() { return mInstance; }

        static uint64_t GetEngineTicks();

        // Managers
        inline managers::RenderManager& GetRenderManager() { return mRenderManager; }
        inline managers::UiManager& GetUiManager() { return mUiManager; }

        // Memory
    	inline based::memory::Arena* GetEngineArena() { return &mEngineArena; }
        inline based::memory::Arena* GetFrameArena() { return &mFrameArena; }
        inline based::memory::Arena* GetGameArena() { return &mGameArena; }
    private:
        void GetInfo();

        [[nodiscard]] bool Initialize();
        void Shutdown();
        void Update(float deltaTime);
        void Render();

        bool mIsRunning;
        bool mIsInitialized;

        core::Window mWindow;
        App* mApp;

        // Managers
        managers::LogManager mLogManager;
        managers::RenderManager mRenderManager;
        managers::UiManager mUiManager;

        // Singleton
        Engine();
        static Engine* mInstance;

        // Arenas
        memory::Arena mEngineArena;
        memory::Arena mGameArena;
        memory::Arena mFrameArena;
        memory::Arena mSceneArena;
    };
}

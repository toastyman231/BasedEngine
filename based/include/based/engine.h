#pragma once

#include "core/window.h"
#include "managers/logmanager.h"
#include "managers/rendermanager.h"

namespace based
{
    class App;
    class Engine
    {
    public:
        static Engine& Instance();
        ~Engine() {}

        void Run(App* app);
        void SetIcon(std::string path);
        inline void Quit() { mIsRunning = false; }

        inline App& GetApp() { return *mApp; }
        inline core::Window& GetWindow() { return mWindow; }

        // Managers
        inline managers::RenderManager& GetRenderManager() { return mRenderManager; }
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

        // Singleton
        Engine();
        static Engine* mInstance;
    };
}
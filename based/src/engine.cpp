#include "engine.h"

#include "log.h"
#include "app.h"
#include "graphics/defaultassetlibraries.h"

#include "input/mouse.h"
#include "input/keyboard.h"
#include "input/joystick.h"

#include "SDL2/SDL.h"
#include <SDL2/SDL_ttf.h>
#include "ui/textentity.h"

namespace based
{
    //public
    Engine& Engine::Instance()
    {
        if (!mInstance)
        {
            mInstance = new Engine();
        }

        return *mInstance;
    }

    //private

    void Engine::Update()
    {
        mWindow.PumpEvents();
        mApp->Update();
    }

    void Engine::Render()
    {
        mWindow.BeginRender();

        mApp->GetCurrentScene()->RenderScene();

        mApp->Render();

        mWindow.EndRender();
    }

    void Engine::GetInfo()
    {
        BASED_TRACE("BasedEngine v{}.{}", 0, 1);
        #ifdef BASED_CONFIG_DEBUG
            BASED_DEBUG("Configuration: DEBUG");
        #endif
        #ifdef BASED_CONFIG_RELEASE
            BASED_DEBUG("Configuration: RELEASE");
        #endif
        #ifdef BASED_PLATFORM_WINDOWS
            BASED_WARN("Platform: Windows");
        #endif
        #ifdef BASED_PLATFORM_LINUX
            BASED_WARN("Platform: Linux");
        #endif
        #ifdef BASED_PLATFORM_MAC
            BASED_WARN("Platform: Mac");
        #endif
    }

    void Engine::Run(App* app)
    {
        mLogManager.Initialize();
        BASED_ASSERT(!mApp, "Attempting to call Engine::Run when a valid app already exists!");
        if (mApp) return;

        mApp = app;
        if (Initialize())
        {
            // Core loop
            while (mIsRunning)
            {
                Update();
                Render();
            }

            Shutdown();
        }
    }

    bool Engine::Initialize()
    {
        BASED_ASSERT(!mIsInitialized, "Attempting to call Engine::Initialize() more than once!");
        bool ret = false;
        if (!mIsInitialized)
        {
            GetInfo();

            if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
            {
                BASED_ERROR("Error initializing SDL2: {}", SDL_GetError());
            }
            else
            {
                TTF_Init();
                SDL_version version;
                SDL_VERSION(&version);
                BASED_INFO("SDL {}.{}.{}", (int32_t)version.major, (int32_t)version.minor, (int32_t)version.patch);

                core::WindowProperties props = mApp->GetWindowProperties();
                if (mWindow.Create(props))
                {
                    // Initialize Managers
                    mRenderManager.Initialize();

                    ret = true;
                    mIsRunning = true;
                    mIsInitialized = true;

                    // Initialize input
                    input::Mouse::Initialize();
                    input::Keyboard::Initialize();

                    // Initialize Asset Libraries
                    graphics::DefaultLibraries::InitializeLibraries();

                    // Initialize glText
                    //ui::TextEntity::Initialize();

                    // Initialize client
                    mApp->Initialize();
                }
            }

            if (!ret)
            {
                BASED_ERROR("Engine Initialization failed. Shutting down.");
                Shutdown();
            }
        }

        return ret;
    }

    void Engine::Shutdown()
    {
        mIsRunning = false;
        mIsInitialized = false;

        // Shutdown client
        mApp->Shutdown();

        // Shutdown glText
        //ui::TextEntity::Terminate();

        // Shutdown input
        input::Joystick::Shutdown();

        // Managers - shutdown in reverse order
        mRenderManager.Shutdown();

        // Shutdown SDL
        mWindow.Shutdown();
        TTF_Quit();
        SDL_Quit();

        mLogManager.Shutdown();
    }

    //Singleton
    Engine* Engine::mInstance = nullptr;

    Engine::Engine() 
        : mIsRunning(false) 
        , mIsInitialized(false)
        , mApp(nullptr)
    {}
}

#include "engine.h"

#include "log.h"

#include "SDL2/SDL.h"

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

    void Engine::Run()
    {
        if (Initialize())
        {
            // Core loop
            while (mIsRunning)
            {
                mWindow.PumpEvents();

                mWindow.BeginRender();
                mWindow.EndRender();
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
            mLogManager.Initialize();
            GetInfo();

            if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
            {
                BASED_ERROR("Error initializing SDL2: {}", SDL_GetError());
            }
            else
            {
                SDL_version version;
                SDL_VERSION(&version);
                BASED_INFO("SDL {}.{}.{}", (int32_t)version.major, (int32_t)version.minor, (int32_t)version.patch);

                if (mWindow.Create())
                {
                    ret = true;
                    mIsRunning = true;
                    mIsInitialized = true;
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

        // Managers - shutdown in reverse order
        mLogManager.Shutdown();

        // Shutdown SDL
        mWindow.Shutdown();
        SDL_Quit();
    }

    //Singleton
    Engine* Engine::mInstance = nullptr;

    Engine::Engine() 
        : mIsRunning(false) 
        , mIsInitialized(false)
    {}
}
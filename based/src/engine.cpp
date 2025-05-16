#include "pch.h"
#include "engine.h"

#include "app.h"
#include "graphics/defaultassetlibraries.h"

#include "input/mouse.h"
#include "input/keyboard.h"
#include "input/joystick.h"

#include <external/stb/stb_image.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "scene/audio.h"

#include "basedtime.h"
#include "graphics/linerenderer.h"
#include "math/basedmath.h"

#ifdef BASED_PLATFORM_WINDOWS
#include <windows.h>
#else
#include <unistd.h>
#include <limits.h>
#endif

constexpr size_t PATH_MAX = 50;

bool logging_enabled = false;

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

    void Engine::Update(float deltaTime)
    {
        mWindow.PumpEvents();
        mInputManager.Update();
        mApp->GetCurrentScene()->UpdateScene(deltaTime);
        mApp->Update(deltaTime);
        mUiManager.Update();
    }

    void Engine::Render()
    {
        mWindow.BeginRender();

        for (const auto& pass : mRenderManager.GetRenderPasses())
        {
            pass->BeginRender();
            GetRenderManager().Submit(BASED_SUBMIT_RC(UpdateGlobals));
            pass->Render();
            pass->EndRender();
            mRenderManager.IncrementPassCount();
        }
        mRenderManager.ResetPassCount();

        mRenderManager.Submit(BASED_SUBMIT_RC(UpdateGlobals));
        mRenderManager.Submit(BASED_SUBMIT_RC(PushFramebuffer, mWindow.GetFramebuffer(), "UserRender", false));
        mRenderManager.Submit(BASED_SUBMIT_RC(PushCamera, mApp->GetCurrentScene()->GetActiveCamera()));

        mApp->Render();
        graphics::DebugLineRenderer::DrawLines();
        mPhysicsManager.DrawDebugBodies();

        mRenderManager.Submit(BASED_SUBMIT_RC(PopCamera));
        mRenderManager.Submit(BASED_SUBMIT_RC(PopFramebuffer));
        mRenderManager.Flush();

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
                PROFILER_FRAME_MARK();
                core::Time::UpdateUnscaledTime();
                const float unscaledTimeDelta = core::Time::GetUnscaledTime() - core::Time::mUnscaledLastFrameTime;
                core::Time::SetUnscaledDelta(unscaledTimeDelta);
                core::Time::UpdateTime();
                const float timeDelta = core::Time::GetTime() - core::Time::mLastFrameTime;
                core::Time::SetDelta(timeDelta);

                // Physics Step
                mPhysicsManager.Update(timeDelta);

                Update(timeDelta);
                Render();
                mJobManager.Flush();
                ClearArena(&mFrameArena);
            }

            Shutdown();
        }
    }

    void Engine::SetIcon(std::string path)
    {
        if (FILE* file = fopen(path.c_str(), "r"))
        {
            int width = 0, height = 0;
            int numChannels = 0;
            Uint32 rmask, gmask, bmask, amask;

            rmask = 0x000000ff;
            gmask = 0x0000ff00;
            bmask = 0x00ff0000;
            amask = 0xff000000;

            stbi_set_flip_vertically_on_load(false);
            unsigned char* pixels = stbi_load(path.c_str(), &width, &height, &numChannels, 0);

            SDL_Surface* iconSurface = SDL_CreateRGBSurfaceFrom(pixels, width, height, 32, width * numChannels, rmask, gmask, bmask, amask);

            SDL_SetWindowIcon(Engine::Instance().GetWindow().GetSDLWindow(), iconSurface);

            delete pixels;
        }
        else
        {
            BASED_WARN("No icon.ico image found at path {}", path.c_str());
        }
    }

    void Engine::SetArgs(int argc, char* argv[])
    {
        if (mNumArgs != -99) return;

	    mNumArgs = argc;
	    for (int index{}; index != argc; ++index)
	    {
	        //BASED_TRACE("Placing back arg {}", argv[index]);
	        mArgs.emplace_back(argv[index]);
	    }
    }

    std::string Engine::GetArg(int index)
    {
        if (index < 0 || index > mNumArgs - 1)
        {
            BASED_WARN("Cannot access arg index {}! Out of range {}!", index, mNumArgs);
            return "";
        }

        return mArgs[index];
    }

    uint64_t Engine::GetEngineTicks()
    {
        return SDL_GetTicks64();
    }

    std::string Engine::GetEngineDirectory()
    {
        if (!mEngineLocation.empty()) return mEngineLocation;

        mEngineLocation = getenv("BASED_ENGINE_HOME");

        return mEngineLocation;
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

                // Initialize memory
                uint32_t engineMemorySize = MEGABYTES_BYTES(2);
                void* engineMemory = malloc(engineMemorySize);
                memset(engineMemory, 0, engineMemorySize);
                mEngineArena = memory::CreateArena(engineMemory, engineMemorySize, "Engine Memory");
                memory::Arena* engineArena = &mEngineArena;
                constexpr uint32_t sceneMemoryPercent = 40;
                uint32_t sceneMemorySize = based::math::PercentOf(engineMemorySize, sceneMemoryPercent);
                constexpr uint32_t frameMemoryPercent = 40;
                uint32_t frameMemorySize = based::math::PercentOf(engineMemorySize, frameMemoryPercent);
                mSceneArena = memory::CreateArena(ArenaAlloc(engineArena, sceneMemorySize), sceneMemorySize, "Scene Memory");
                mFrameArena = memory::CreateArena(ArenaAlloc(engineArena, frameMemorySize), frameMemorySize, "Frame Memory");

                uint32_t gameMemorySize = Instance().GetApp().GetGameSettings().gameMemory;
                void* gameMemory = malloc(gameMemorySize);
                memset(gameMemory, 0, gameMemorySize);
                mGameArena = memory::CreateArena(gameMemory, gameMemorySize, "Game Memory");

                core::WindowProperties props = mApp->GetWindowProperties();
                if (mWindow.Create(props))
                {
                    // Initialize Managers
                    mRenderManager.Initialize();
                    mJobManager.Initialize();
                    mResourceManager.Initialize();
                    mInputManager.Initialize();

                    ret = true;
                    mIsRunning = true;
                    mIsInitialized = true;

                    // Initialize input
                    input::Mouse::Initialize();
                    input::Keyboard::Initialize();

                    // Initialize Asset Libraries
                    graphics::DefaultLibraries::InitializeLibraries();

                    // Initialize passes
                    auto computePass = new graphics::ComputePass("Compute Dispatches");
                    mRenderManager.InjectPass(computePass);
                    auto shadowDepthPass = new graphics::CustomRenderPass(
                        "ShadowDepthPass", mWindow.GetShadowBuffer(),
                        graphics::DefaultLibraries::GetMaterialLibrary().Get("ShadowDepthMaterial"));
                    shadowDepthPass->AddOutputName("ShadowMap");
                    mRenderManager.InjectPass(shadowDepthPass);
                    auto mainRenderPass = new graphics::CustomRenderPass(
                        "MainColorPass", mWindow.GetFramebuffer());
                    mainRenderPass->AddOutputName("SceneColor");
                    mainRenderPass->AddOutputName("SceneDepth");
                    mRenderManager.InjectPass(mainRenderPass);
                    auto uiRenderPass = new graphics::UIRenderPass("UserInterfacePass", mWindow.GetFramebuffer());
                    uiRenderPass->AddOutputName("SceneColor");
                    mRenderManager.InjectPass(uiRenderPass);

                    // Initialize UI after input and rendering is initialized
                    mUiManager.Initialize();

                    // Load window icon
                    SetIcon("Assets/icon.ico");

                    //Initialize miniaudio
                    scene::Audio::InitEngine();

                    // Initialize buffer objects
                    graphics::Shader::InitializeUniformBuffers();

                    // Initialize physics system
                    mPhysicsManager.Initialize();

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
        mApp->GetCurrentScene()->ShutdownScene();
        mApp->Shutdown();

        // Shutdown Physics
        mPhysicsManager.Shutdown();

        // Shutdown Audio
        scene::Audio::ShutdownEngine();

        // Shutdown UI
        mUiManager.Shutdown();

        // Shutdown libraries
        graphics::DefaultLibraries::UninitializeLibraries();

        // Shutdown input
        input::Joystick::Shutdown();

        // Managers - shutdown in reverse order
        mInputManager.Shutdown();
        mResourceManager.Shutdown();
        mJobManager.Shutdown();
        mRenderManager.Shutdown();

        // Shutdown SDL
        mWindow.Shutdown();
        TTF_Quit();
        SDL_Quit();

        mLogManager.Shutdown();

        ArenaFreeAll(&mEngineArena);
        ArenaFreeAll(&mGameArena);
    }

    //Singleton
    Engine* Engine::mInstance = nullptr;

    Engine::Engine() 
        : mIsRunning(false) 
        , mIsInitialized(false)
        , mApp(nullptr)
    {}
}

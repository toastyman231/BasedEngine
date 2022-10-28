#include "engine.h"

#include "log.h"
#include "graphics/mesh.h"
#include "graphics/shader.h"

#include "input/mouse.h"
#include "input/keyboard.h"

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
            {
                // Test mesh
                float vertices[]
                {
                     0.5,  0.5f, 0.f,
                     0.5, -0.5f, 0.f,
                    -0.5, -0.5f, 0.f,
                    -0.5,  0.5f, 0.f,
                };
                uint32_t elements[]
                {
                    0, 3, 1,
                    1, 3, 2
                };
                std::shared_ptr<graphics::Mesh> mesh = std::make_shared<graphics::Mesh>(&vertices[0], 4, 3, &elements[0], 6);

                // Test shader
                const char* vertexShader = R"(
                    #version 410 core
                    layout (location = 0) in vec3 position;
                    out vec3 vpos;
                    uniform vec2 offset = vec2(0.5);
                    void main()
                    {
                        vpos = position + vec3(offset, 0);
                        gl_Position = vec4(position, 1.0);
                    }
                )";
                const char* fragmentShader = R"(
                    #version 410 core
                    out vec4 outColor;
                    in vec3 vpos;

                    uniform vec3 color = vec3(0.0);
                    void main()
                    {
                        outColor = vec4(vpos, 1.0);
                    }
                )";
                std::shared_ptr<graphics::Shader> shader = std::make_shared<graphics::Shader>(vertexShader, fragmentShader);
                shader->SetUniformFloat3("color", 1, 0, 0);

                float xKeyOffset = 0.f;
                float yKeyOffset = 0.f;
                float keySpeed = 0.0001f;

                // Core loop
                while (mIsRunning)
                {
                    mWindow.PumpEvents();

                    int windowWidth = 0;
                    int windowHeight = 0;
                    GetWindow().GetSize(windowWidth, windowHeight);
                    
                    float xNorm = (float)input::Mouse::X() / (float)windowWidth;
                    float yNorm = (float)(windowHeight - input::Mouse::Y()) / (float)windowHeight;

                    if (input::Keyboard::Key(BASED_INPUT_KEY_LEFT)) xKeyOffset -= keySpeed;
                    if (input::Keyboard::Key(BASED_INPUT_KEY_RIGHT)) xKeyOffset += keySpeed;
                    if (input::Keyboard::Key(BASED_INPUT_KEY_UP)) yKeyOffset += keySpeed;
                    if (input::Keyboard::Key(BASED_INPUT_KEY_DOWN)) yKeyOffset -= keySpeed;

                    if (input::Keyboard::KeyDown(BASED_INPUT_KEY_LEFT)) xKeyOffset -= keySpeed * 100;
                    if (input::Keyboard::KeyDown(BASED_INPUT_KEY_RIGHT)) xKeyOffset += keySpeed * 100;

                    shader->SetUniformFloat2("offset", xNorm + xKeyOffset, yNorm + yKeyOffset);

                    mWindow.BeginRender();

                    auto rc = std::make_unique<graphics::rendercommands::RenderMesh>(mesh, shader);
                    mRenderManager.Submit(std::move(rc));
                    mRenderManager.Flush();

                    mWindow.EndRender();
                }
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
                    // Initialize Managers
                    mRenderManager.Initialize();

                    ret = true;
                    mIsRunning = true;
                    mIsInitialized = true;

                    // Initialize input
                    input::Mouse::Initialize();
                    input::Keyboard::Initialize();
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
        mRenderManager.Shutdown();

        // Shutdown SDL
        mWindow.Shutdown();
        SDL_Quit();

        mLogManager.Shutdown();
    }

    //Singleton
    Engine* Engine::mInstance = nullptr;

    Engine::Engine() 
        : mIsRunning(false) 
        , mIsInitialized(false)
    {}
}
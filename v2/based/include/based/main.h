#pragma once

#include "core/NewDelete.h"

#include <chrono>
#include <cstdio>
#include <format>
#include <random>
#include <thread>

#include "core/BasedLog.h"
#include "core/LogManager.h"
#include "memory/MemoryPoolHeader.h"
#include "memory/PlatformMemUtils.h"

#ifdef PROFILE_MEMORY_LEAKS
#ifdef BASED_CONFIG_DEBUG
#include "vld.h"
#endif
#endif

//#include "engine.h"
//#include "app.h"

// To be implemented in client app
// The client returns a pointer to an instance of a class derived from based::App
// The ownership of the returned pointer belongs to Based, and will be managed as such
//
// Example:
// class ClientApp : public based::App {};
// based::App* CreateApp() { return new ClientApp(); }
//based::App* CreateApp();

#ifdef BASED_CONFIG_RELEASE
#ifdef BASED_PLATFORM_WINDOWS
#include <windows.h>
int WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
#endif
#else
int main(int argc, char* argv[])
#endif
{
#ifdef BASED_CONFIG_RELEASE
#ifdef BASED_PLATFORM_WINDOWS
    int argc = __argc;
    char** argv = __argv;
#endif
#endif

    based::LogManager logMan;
    logMan.Initialize();
    based::BootstrapAllocator::DisableBootstrap();
    
    based::SetupMemoryPools();
    based::AllocatorScope ac(based::ePoolIdentifier::kPersistentPool);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> distrib(16, 100000000);
    size_t totalAllocated = 0;

    while (true) {
        auto startTime = std::chrono::steady_clock::now();

        size_t amount = distrib(gen);

        // Allocate out of your custom wrapper instance
        void* pTemp = new uint8[amount];

        if (!pTemp) {
            BASED_ERROR("Pool allocation failed or reached absolute limit trying to allocate {}! Total: {}",
                MemSize{amount}, MemSize{totalAllocated});
            break;
        }

        // CRITICAL: Overwrite the entirety of the 100MB block to register it on the terminal telemetry
        std::memset(pTemp, 0xCC, amount);

        totalAllocated += amount;
        BASED_TRACE("Allocated {:2.1} at {}. Total: {:2.1}", MemSize{amount}, pTemp, MemSize{totalAllocated});

        auto endTime = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        auto sleepTime = std::chrono::milliseconds(10) - elapsed;

        if (sleepTime.count() > 0) {
            std::this_thread::sleep_for(sleepTime);
        }
    }
    
    /*based::App* app = CreateApp();
    based::Engine::Instance().SetArgs(argc, argv);
    based::Engine::Instance().Run(app);

    delete app;
    delete based::Engine::GetRawEngineInstance();*/

    return 0;
}
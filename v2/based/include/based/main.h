#pragma once

#ifndef _SILENCE_CXX20_OLD_SHARED_PTR_ATOMIC_SUPPORT_DEPRECATION_WARNING
#define _SILENCE_CXX20_OLD_SHARED_PTR_ATOMIC_SUPPORT_DEPRECATION_WARNING
#endif

#include "memory/MemoryPoolHeader.h"
#include "memory/PlatformMemUtils.h"
#include "core/LogManager.h"
#include "core/FileSystem.h"
#include "core/App.h"
#include "tsoding/flag.h"
#include "Engine.h"

#ifdef PROFILE_MEMORY_LEAKS
#ifdef BASED_CONFIG_DEBUG
#include "vld.h"
#endif
#endif

void usage(FILE *stream)
{
    fprintf(stream, "Usage: ./example [OPTIONS] [--] [ARGS]\n");
    fprintf(stream, "OPTIONS:\n");
    flag_print_options(stream);
}

void print_list(const char **items, size_t count)
{
    printf("[");
    for (size_t i = 0; i < count; ++i) {
        if (i > 0) printf(", ");
        printf("%s", items[i]);
    }
    printf("]\n");
}

// To be implemented in client app
// The client returns a pointer to an instance of a class derived from based::App
// The ownership of the returned pointer belongs to Based, and will be managed as such
//
// Example:
// class ClientApp : public based::App {};
// based::App* CreateApp() { return new ClientApp(); }
namespace based
{
    App* CreateApp();
}

#ifdef BASED_CONFIG_RELEASE
#ifdef BASED_PLATFORM_WINDOWS
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
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

    based::LogManager::Initialize();
    based::BootstrapAllocator::DisableBootstrap();
    
    based::SetupMemoryPools();
    based::SetupThirdPartyMemoryCallbacks();
    based::AllocatorScope ac(based::ePoolIdentifier::kPersistentPool);

    const bool bSuccess = based::InitializeRootFileSystem();
    BASED_ASSERT_FMT(bSuccess, "Couldn't set up root filesystem, is root path '{}' valid?",
        based::GetExecutableDirectory());
    
    based::App* app = based::CreateApp();
    BASED_ASSERT(app, "Invalid app was returned!");

    based::Engine::Instance().SetArgs(argc, argv);
    based::Engine::Instance().Run(app);

    delete app;

    return 0;
}
#include <cstdio>

#include "../../external/Private/mimalloc/include/mimalloc.h"

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

// IMPORTANT!! NEEDED FOR MIMALLOC_REDIRECT ON WINDOWS!
#if defined(_WIN32) && defined(MI_SHARED_LIB)
#pragma comment(linker, "/INCLUDE:mi_allocator_init")
#endif

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
    
    mi_version();

    void* p = malloc(1024);
    printf("0x%p\n", p);
    free(p);
    printf("Hello World!");
    
    /*based::App* app = CreateApp();
    based::Engine::Instance().SetArgs(argc, argv);
    based::Engine::Instance().Run(app);

    delete app;
    delete based::Engine::GetRawEngineInstance();*/

    return 0;
}
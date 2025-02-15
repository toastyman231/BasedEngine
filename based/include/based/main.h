#pragma once

#ifdef PROFILE_MEMORY_LEAKS
#ifdef BASED_CONFIG_DEBUG
#include "vld.h"
#endif
#endif

#include <iostream>

#include "engine.h"
#include "app.h"

// To be implemented in client app
// The client returns a pointer to an instance of a class derived from based::App
// The ownership of the returned pointer belongs to Based, and will be managed as such
//
// Example:
// class ClientApp : public based::App {};
// based::App* CreateApp() { return new ClientApp(); }
based::App* CreateApp();

int main(int argc, char* argv[])
{
    based::App* app = CreateApp();
    based::Engine::Instance().SetArgs(argc, argv);
    based::Engine::Instance().Run(app);

    delete app;
    delete based::Engine::GetRawEngineInstance();

    return 0;
}
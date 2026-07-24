#pragma once

#include "Window.h"

namespace based
{
    struct AppInfo final
    {
        std::string_view appName;

        AppInfo()
            : appName("Based App")
        {}
    };
    
    class App : public NonMoveable
    {
        friend class Engine;
    public:
        App() = default;
        virtual ~App() = default;
        
        virtual void Initialize() {}
        virtual void Update(float fDeltaTime) {}
        virtual void Render() {}
        virtual void Shutdown() {}

        virtual bool ValidateShutdown() { return true; }
        virtual void HandleFileDrop(const std::string& path) {}

        virtual WindowProperties GetWindowProperties() { return {}; }
        virtual AppInfo GetAppInfo() { return {}; }

    private:
        // Only intended to be called by Engine::DeclareFlags()
        virtual void DeclareFlags() {}
        virtual void PrintFlagUsage() {}
    };
}

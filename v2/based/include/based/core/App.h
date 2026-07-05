#pragma once

namespace based
{
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

    private:
        // Only intended to be called by Engine::DeclareFlags()
        virtual void DeclareFlags() {}
    };
}

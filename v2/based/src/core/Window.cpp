#include "pch.h"
#include "core/Window.h"
#include "core/PlatformWindow.h"

#include "memory/MemoryPoolHeader.h"
#include "SDL3/SDL_video.h"

namespace based
{
    WindowProperties::WindowProperties()
    {
        title = "BasedApp";
        w = 1920;
        h = 1080;
        wMin = 320;
        hMin = 180;
        flags = SDL_WINDOW_RESIZABLE;
        fAspectRatio = 16.f / 9.f;
        clearColor = Vec3(
            static_cast<float>(0x64) / static_cast<float>(0xFF),
            static_cast<float>(0x95) / static_cast<float>(0xFF),
            static_cast<float>(0xED) / static_cast<float>(0xFF));
    }

    IWindow* WindowFactory::Create(const WindowProperties& props)
    {
        AllocatorScope ac(ePoolIdentifier::kPersistentPool);
        IWindow* pWindow = new PlatformWindow();
        pWindow->m_WindowProps = props;
        const bool bSuccess = pWindow->Create_Internal();
        BASED_ASSERT(bSuccess, "Failed to create window!");
        return pWindow;
    }

    void* WindowFactory::CreateAndGetWindowSurface(IWindow* pWindow)
    {
        AllocatorScope ac(ePoolIdentifier::kPersistentPool);
        pWindow->CreateSurface();
        BASED_ASSERT(pWindow->GetSurface(), "Failed to create surface!");
        return pWindow->GetSurface();
    }
}

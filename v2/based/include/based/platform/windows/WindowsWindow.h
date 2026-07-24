#pragma once

#ifndef BASED_PREVENT_ACCIDENTAL_WINDOW_INCLUDES
#error "Don't include this directly! Include Window.h instead and that will bring this in."
#endif

#include "core/Window.h"

struct SDL_Window;
struct SDL_Cursor;

#ifdef BASED_USE_NAMESPACE
namespace based
{
#endif
    class WindowsWindow final : protected IWindow
    {
        friend class WindowFactory;
    public:
        void Shutdown() override;
        void PumpEvents() override;
        IVec2 GetSize() override;
        void* GetNativeHandle() override;
        void* GetSurface() const override;
        void SetWindowTitle(const std::string& pStrTitle) override;
        void SetWindowFullscreen(eFullscreenMode mode) override;
        void SetCursor(eCursorMode mode) const;
    private:
        WindowsWindow() : IWindow() {}
        ~WindowsWindow() override {}

        bool Create_Internal() override;
        void* CreateSurface() override;

        SDL_Window* m_pSystemWindow = nullptr;
        void* m_pSurface = nullptr;

        std::array<SDL_Cursor*, kCursorCount> m_pCursors = {};
    };
#ifdef BASED_USE_NAMESPACE
}
#endif

#include "pch.h"
#include "core/Window.h"
#define BASED_PREVENT_ACCIDENTAL_WINDOW_INCLUDES
#define BASED_USE_NAMESPACE
#include "platform/windows/WindowsWindow.h"

#include "Engine.h"
#include "core/App.h"
#include "memory/MemoryPoolHeader.h"
#include "SDL3/SDL_events.h"
#include "SDL3/SDL_init.h"
#include "SDL3/SDL_mouse.h"
#include "SDL3/SDL_version.h"
#include "SDL3/SDL_video.h"

namespace based
{
    bool WindowsWindow::Create_Internal()
    {
        SDL_SetMemoryFunctions(my_malloc, my_calloc, my_realloc, my_free);
        AllocatorScope ac(ePoolIdentifier::kPersistentPool);

        // Currently this and Shutdown assume only one window, so we init and shutdown SDL itself alongside that
        // one window. If/when we want more than one window, we can add a Destroy function that only destroys the
        // window without shutting down SDL, and only init SDL on the first window creation. 
        const bool bSuccess = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_HAPTIC | SDL_INIT_EVENTS);
        if (!bSuccess)
        {
            BASED_ERROR("Error initializing SDL: {}", SDL_GetError());
            return false;
        }
        
        BASED_INFO("SDL {}.{}.{}", SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_MICRO_VERSION);
        
        m_pSystemWindow = SDL_CreateWindow(
            m_WindowProps.title.data(),
            m_WindowProps.w, m_WindowProps.h,
            m_WindowProps.flags | SDL_WINDOW_VULKAN);
        if (!m_pSystemWindow)
        {
            BASED_ERROR("Error creating window: {}", SDL_GetError());
            return false;
        }

        SDL_SetWindowMinimumSize(m_pSystemWindow, m_WindowProps.wMin, m_WindowProps.hMin);

        m_pCursors[to_underlying(eCursorMode::kDefault)] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_DEFAULT);
        m_pCursors[to_underlying(eCursorMode::kMove)] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_MOVE);
        m_pCursors[to_underlying(eCursorMode::kPointer)] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_POINTER);
        m_pCursors[to_underlying(eCursorMode::kResize)] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_NWSE_RESIZE);
        m_pCursors[to_underlying(eCursorMode::kCross)] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_CROSSHAIR);
        m_pCursors[to_underlying(eCursorMode::kText)] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_TEXT);
        m_pCursors[to_underlying(eCursorMode::kUnavailable)] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_NOT_ALLOWED);

        return true;
    }
    
    void WindowsWindow::Shutdown()
    {
        AllocatorScope ac(ePoolIdentifier::kPersistentPool);
        
        for (const auto pCursor : m_pCursors)
        {
            SDL_DestroyCursor(pCursor);
        }
        
        SDL_DestroyWindow(m_pSystemWindow);
        m_pSystemWindow = nullptr;

        SDL_Quit();
    }
    
    void WindowsWindow::PumpEvents()
    {
        AllocatorScope ac(ePoolIdentifier::kPersistentPool);
        
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_EVENT_QUIT:
                if (Engine::Instance().GetApp().ValidateShutdown())
                    Engine::Instance().Quit();
                break;
            // TODO: Handle controller events and resizing
            default:
                break;
            }
        }
    }
    
    IVec2 WindowsWindow::GetSize()
    {
        int w, h;
        SDL_GetWindowSize(m_pSystemWindow, &w, &h);
        return {w, h};
    }
    
    void WindowsWindow::SetWindowTitle(const std::string& pStrTitle)
    {
        SDL_SetWindowTitle(m_pSystemWindow, pStrTitle.c_str());
        m_WindowProps.title = pStrTitle;
    }
    
    void WindowsWindow::SetWindowFullscreen(eFullscreenMode mode)
    {
        AllocatorScope ac(ePoolIdentifier::kPersistentPool);
        
        SDL_DisplayID display = SDL_GetDisplayForWindow(m_pSystemWindow);

        switch (mode)
        {
        case eFullscreenMode::kNone:
            {
                SDL_SetWindowFullscreen(m_pSystemWindow, false);
                SDL_SetWindowBordered(m_pSystemWindow, true);
                break;
            }

        case eFullscreenMode::kBorderlessFullscreen:
            {
                SDL_SetWindowFullscreen(m_pSystemWindow, false);
                SDL_SetWindowBordered(m_pSystemWindow, false);

                SDL_Rect bounds;
                SDL_GetDisplayBounds(display, &bounds);

                SDL_SetWindowPosition(m_pSystemWindow, bounds.x, bounds.y);
                SDL_SetWindowSize(m_pSystemWindow, bounds.w, bounds.h);
                break;
            }

        case eFullscreenMode::kExclusiveFullscreen:
            {
                const SDL_DisplayMode* pMode = SDL_GetDesktopDisplayMode(display);

                SDL_SetWindowFullscreenMode(m_pSystemWindow, pMode);
                SDL_SetWindowFullscreen(m_pSystemWindow, true);
                break;
            }
        }
    }
    
    void WindowsWindow::SetCursor(eCursorMode mode) const
    {
        SDL_SetCursor(m_pCursors[to_underlying(mode)]);
    }
}

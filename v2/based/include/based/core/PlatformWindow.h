#pragma once

namespace based
{
#define BASED_PREVENT_ACCIDENTAL_WINDOW_INCLUDES
#ifdef BASED_PLATFORM_WINDOWS
#include "platform/windows/WindowsWindow.h"
    using PlatformWindow = WindowsWindow;
#else
#error "You need to implement IWindow for this platform and typedef PlatformWindow!"
#endif
#undef BASED_PREVENT_ACCIDENTAL_WINDOW_INCLUDES
}

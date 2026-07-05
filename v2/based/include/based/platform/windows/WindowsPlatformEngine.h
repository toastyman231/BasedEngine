#pragma once

namespace based
{
    class WindowsPlatformEngine final
    {
    public:
        static uint64 GetEngineTicks();
        static const std::string& GetPlatformName();
    };

    using PlatformEngine = WindowsPlatformEngine;
}

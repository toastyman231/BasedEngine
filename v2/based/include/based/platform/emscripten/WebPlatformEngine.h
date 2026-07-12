#pragma once

namespace based
{
    class WebPlatformEngine final
    {
    public:
        static uint64 GetEngineTicks();
        static const std::string& GetPlatformName();
    };

    using PlatformEngine = WebPlatformEngine;
}

#pragma once

#include <spdlog/spdlog.h>
#include <cstdint>

namespace based
{
    inline spdlog::logger* g_pDefaultLogger = nullptr;
    
    class LogManager final
    {
    public:
        LogManager() = default;
        ~LogManager() = default;

        void Initialize();
        void Shutdown();
    };
}

// fmt custom formatter to display memory sizes nicely

struct MemSize {
    uint64_t bytes;
};
 
template <>
struct fmt::formatter<MemSize> {
    int precision  = 2;  // decimal places
    int padWidth   = 0;  // minimum digits left of decimal, 0 = no padding

    constexpr auto parse(fmt::format_parse_context& ctx) {
        auto it = ctx.begin();

        // Parse first number
        if (it != ctx.end() && *it >= '0' && *it <= '9') {
            int first = *it++ - '0';

            // If followed by a dot, first number is padWidth
            if (it != ctx.end() && *it == '.') {
                ++it; // skip dot
                padWidth  = first;
                if (it != ctx.end() && *it >= '0' && *it <= '9')
                    precision = *it++ - '0';
            } else {
                // No dot — single number is just precision, same as before
                precision = first;
            }
        }

        if (it != ctx.end() && *it != '}') assert(false && "Invalid MemSize format spec!");

        return it;
    }

    auto format(const MemSize& m, fmt::format_context& ctx) const {
        static constexpr struct { uint64_t threshold; const char* suffix; } kUnits[] = {
            { 1ULL << 60, " EiB" },
            { 1ULL << 50, " PiB" },
            { 1ULL << 40, " TiB" },
            { 1ULL << 30, " GiB" },
            { 1ULL << 20, " MiB" },
            { 1ULL << 10, " KiB" },
            { 0,          " B"   },
        };

        for (auto& u : kUnits) {
            if (m.bytes >= u.threshold && u.threshold > 0) {
                double val = static_cast<double>(m.bytes)
                           / static_cast<double>(u.threshold);

                if (padWidth > 0) {
                    // Format with padding on the integer part.
                    // We do this by formatting the whole number with enough
                    // total width to guarantee padWidth digits left of decimal.
                    // total width = padWidth + 1 (dot) + precision
                    int totalWidth = padWidth + (precision > 0 ? 1 + precision : 0);
                    return fmt::format_to(ctx.out(), "{:{}.{}f}{}",
                                         val, totalWidth, precision, u.suffix);
                }

                return fmt::format_to(ctx.out(), "{:.{}f}{}",
                                      val, precision, u.suffix);
            }
        }

        // Plain bytes — apply pad width if requested
        if (padWidth > 0)
            return fmt::format_to(ctx.out(), "{:{}} B", m.bytes, padWidth);

        return fmt::format_to(ctx.out(), "{} B", m.bytes);
    }
};

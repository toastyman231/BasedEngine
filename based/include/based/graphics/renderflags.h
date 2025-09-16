#pragma once
#include "core/basedtypes.h"

namespace based::graphics
{
    enum class RenderFlags : uint8_t
    {
        None = 0,
        DrawOpaque = 1 << 0,
        DrawMasked = 1 << 1,
        DrawTranslucent = 1 << 2,

        DrawAll = DrawOpaque | DrawMasked | DrawTranslucent,
    };
    DEFINE_ENUM_CLASS_BITWISE_OPERATORS(RenderFlags)
}

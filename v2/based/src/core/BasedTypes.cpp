#include "pch.h"
#include "core/BasedTypes.h"

namespace based
{
    template <typename From, typename To>
    constexpr From EnumMap<From, To>::map(To value) const
    {
        auto it = std::ranges::find_if(m_table,
            [value](const To& to)
            {
                return to == value;
            });
        if (it != m_table.end())
        {
            return static_cast<From>(std::distance(m_table, it));
        }

        BASED_ASSERT_FMT(false, "Could not find a value for {}!", to_underlying(value));
        return static_cast<From>(0);
    }
}

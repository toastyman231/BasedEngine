#pragma once

namespace based
{
    template <typename From, typename To>
    class EnumMap final
    {
        static constexpr size_t Size = static_cast<size_t>(From::kCount);
    public:

        struct Mapping
        {
            From engine_value;
            To api_value;
        };

        static constexpr bool ValidateCount(const std::array<uint32, Size>& counts)
        {
            for (size_t i = 0; i < Size; ++i)
            {
                if (counts[i] != 1) return false;
            }
            return true;
        }

        template <size_t N>
        consteval EnumMap(const std::array<Mapping, N>& mappings)
        {
            std::array<uint32, Size> hit_counts{};

            for (size_t i = 0; i < N; ++i)
            {
                size_t index = static_cast<size_t>(mappings[i].engine_value);

                if (index < Size)
                {
                    m_table[index] = mappings[i].api_value;
                    ++hit_counts[index];
                }
            }

            if (!ValidateCount(hit_counts))
            {
                detail::InvalidCompileTimeOperation();
            }
        }

        constexpr To map(From value) const
        {
            return m_table[static_cast<size_t>(value)];
        }

        constexpr From map(To value) const
        {
            auto it = std::ranges::find_if(m_table,
            [value](const To& to)
            {
                return to == value;
            });
            if (it != m_table.end())
            {
                return static_cast<From>(std::distance(m_table.begin(), it));
            }

            BASED_ASSERT_FMT(false, "Could not find a value for {}!", to_underlying(value));
            return static_cast<From>(0);
        }
        
    private:
        std::array<To, Size> m_table{};
    };
}

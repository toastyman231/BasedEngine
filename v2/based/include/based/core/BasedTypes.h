#pragma once

#include <concepts>
#include <type_traits>

using uint8 = uint8_t;
using uint16 = uint16_t;
using uint32 = uint32_t;
using uint64 = uint64_t;

using int8 = int8_t;
using int16 = int16_t;
using int32 = int32_t;
using int64 = int64_t;

namespace based
{
    struct NonCopyable
    {
        NonCopyable() = default;
        
        NonCopyable(const NonCopyable& other) = delete;
        NonCopyable& operator=(const NonCopyable& other) = delete;

        NonCopyable(NonCopyable&& other) = default;
        NonCopyable& operator=(NonCopyable&& other) = default;
    };

    // Stuff like std::mutex where it's address is part of its value can't be moved, so I'll have this for that scenario
    // It does mess with the ability to do factories and containers like std::vector though
    struct NonMoveable
    {
        NonMoveable() = default;

        // There's really no reason to allow something to be copyable and not moveable.
        // Allowing copies would mean that std::move(myNonMoveable) would compile but silently perform a copy
        // instead of a move. I think that case should just not compile, so I delete copy constructor/assignment.
        NonMoveable(const NonMoveable& other) = delete;
        NonMoveable& operator=(const NonMoveable& other) = delete;

        NonMoveable(NonMoveable&& other) = delete;
        NonMoveable& operator=(NonMoveable&& other) = delete;
    };

    // This is so I can Rule of 5 a class without having to write all this out
    // Yes it's also moveable, but NonMoveable kind of conflicts with this
    struct ExplicitlyCopyable
    {
        ExplicitlyCopyable() = default;

        ExplicitlyCopyable(const ExplicitlyCopyable& other) = default;
        ExplicitlyCopyable& operator=(const ExplicitlyCopyable& other) = default;

        ExplicitlyCopyable(ExplicitlyCopyable&& other) = default;
        ExplicitlyCopyable& operator=(ExplicitlyCopyable&& other) = default;
    };

    // C++23 Type Traits back-ported to C++20

#if __cplusplus == 202002L || (defined(_MSVC_LANG) && _MSVC_LANG == 202002L)
    template <typename T>
    struct is_scoped_enum {
        static constexpr bool value = 
            std::is_enum_v<T> && 
            !std::is_convertible_v<T, std::underlying_type_t<T>>;
    };

    template <typename T>
    inline constexpr bool is_scoped_enum_v = is_scoped_enum<T>::value;

    template <typename Enum> requires std::is_enum_v<Enum>
    [[nodiscard]] constexpr std::underlying_type_t<Enum> to_underlying(Enum e) noexcept 
    {
        return static_cast<std::underlying_type_t<Enum>>(e);
    }
#else
#error "If C++ version is >= C++23, you can get rid of these custom type traits!"
#endif

    // General type traits
    
    template <typename E, typename Underlying>
    concept EnumClassWithUnderlying = 
        is_scoped_enum_v<E> && 
        std::same_as<std::underlying_type_t<E>, Underlying>;
    
}

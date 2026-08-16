#pragma once

#if defined(_MSC_VER) && !defined(__clang__)
    #define BASED_WEAK(ReturnType, Name) \
        extern "C" ReturnType Name##_BasedWeakDefault(); \
        __pragma(comment(linker, "/alternatename:" #Name "=" #Name "_BasedWeakDefault")) \
        __pragma(comment(linker, "/include:" #Name "_BasedWeakDefault")) \
        extern "C" ReturnType Name##_BasedWeakDefault()

#elif defined(__GNUC__) || defined(__clang__)
    #define BASED_WEAK(ReturnType, Name) \
        __attribute__((weak)) ReturnType Name()

#else
    #define BASED_WEAK(ReturnType, Name) ReturnType Name()
#endif

// Courtesy of https://voithos.io/articles/enum-class-bitmasks/
#define DEFINE_ENUM_CLASS_BITWISE_OPERATORS(Enum)                   \
    inline constexpr Enum operator|(Enum Lhs, Enum Rhs) {           \
        return static_cast<Enum>(                                   \
            static_cast<std::underlying_type_t<Enum>>(Lhs) |        \
            static_cast<std::underlying_type_t<Enum>>(Rhs));        \
    }                                                               \
    inline constexpr Enum operator&(Enum Lhs, Enum Rhs) {           \
        return static_cast<Enum>(                                   \
            static_cast<std::underlying_type_t<Enum>>(Lhs) &        \
            static_cast<std::underlying_type_t<Enum>>(Rhs));        \
    }                                                               \
    inline constexpr Enum operator^(Enum Lhs, Enum Rhs) {           \
        return static_cast<Enum>(                                   \
            static_cast<std::underlying_type_t<Enum>>(Lhs) ^        \
            static_cast<std::underlying_type_t<Enum>>(Rhs));        \
    }                                                               \
    inline constexpr Enum operator~(Enum E) {                       \
        return static_cast<Enum>(                                   \
            ~static_cast<std::underlying_type_t<Enum>>(E));         \
    }                                                               \
    inline Enum& operator|=(Enum& Lhs, Enum Rhs) {                  \
        return Lhs = static_cast<Enum>(                             \
                   static_cast<std::underlying_type_t<Enum>>(Lhs) | \
                   static_cast<std::underlying_type_t<Enum>>(Lhs)); \
    }                                                               \
    inline Enum& operator&=(Enum& Lhs, Enum Rhs) {                  \
        return Lhs = static_cast<Enum>(                             \
                   static_cast<std::underlying_type_t<Enum>>(Lhs) & \
                   static_cast<std::underlying_type_t<Enum>>(Lhs)); \
    }                                                               \
    inline Enum& operator^=(Enum& Lhs, Enum Rhs) {                  \
        return Lhs = static_cast<Enum>(                             \
                   static_cast<std::underlying_type_t<Enum>>(Lhs) ^ \
                   static_cast<std::underlying_type_t<Enum>>(Lhs)); \
    }

#ifdef BASED_CONFIG_DEBUG
#define BASED_STATIC_ASSERT(x, msg) static_assert((x) && msg)
#else
#define BASED_STATIC_ASSERT(x, msg) (void)0
#endif

// Call this function to immediately throw a compile-time error
namespace detail
{
    [[noreturn]] void InvalidCompileTimeOperation();
}

#pragma once

#if defined(_MSC_VER) && !defined(__clang__)
    // Native Microsoft Visual C++ compiler
    #define BASED_WEAK // MSVC looks in the user application first, so we don't need anything special
#elif defined(__GNUC__) || defined(__clang__)
    // GCC, Standard Clang, or Clang-cl
    #define BASED_WEAK __attribute__((weak))
#else
    #define BASED_WEAK
#endif
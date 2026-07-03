#pragma once

#include <cassert>
#include <new>

#if defined(_WIN32)
    #define BASED_C_ALLOC_OVERRIDE __declspec(dllexport)
#else
    #define BASED_C_ALLOC_OVERRIDE
#endif

// operator new/delete
void* operator new(std::size_t size);
void* operator new[](std::size_t size);
void* operator new(std::size_t size, const std::nothrow_t&) noexcept;
void* operator new[](std::size_t size, const std::nothrow_t&) noexcept;
void* operator new(std::size_t size, std::align_val_t al);
void* operator new[](std::size_t size, std::align_val_t al);
void* operator new(std::size_t size, std::align_val_t al, const std::nothrow_t&) noexcept;
void* operator new[](std::size_t size, std::align_val_t al, const std::nothrow_t&) noexcept;
void  operator delete(void* ptr) noexcept;

// Core C heap
// Overriding these is a huge ass pain on Windows, so we just hope that the libraries we use
// give us a mechanism to supply them ourselves
void* my_malloc(size_t size);
void  my_free(void* ptr);
void* my_realloc(void* ptr, size_t size);
void* my_calloc(size_t num, size_t size);

// C11 aligned allocation
BASED_C_ALLOC_OVERRIDE void* aligned_alloc(size_t alignment, size_t size);

// POSIX aligned allocation
#ifdef BASED_PLATFORM_LINUX
void* malloc(size_t size);
void  free(void* ptr);
void* realloc(void* ptr, size_t size);
void* calloc(size_t num, size_t size);

int posix_memalign(void** ptr, size_t alignment, size_t size);
#endif
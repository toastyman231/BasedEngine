#include "pch.h"
#include "core/NewDelete.h"

#include "memory/MemoryManager.h"
#include "memory/PlatformMemUtils.h"

void* operator new(std::size_t size)
{
    return based::MemoryManager::MemAlign(size, based::GetPlatformDefaultAlignment());
}

void* operator new[](std::size_t size)
{
    return based::MemoryManager::MemAlign(size, based::GetPlatformDefaultAlignment());
}

void* operator new(std::size_t size, const std::nothrow_t&) noexcept
{
    return operator new(size);
}

void* operator new[](std::size_t size, const std::nothrow_t&) noexcept
{
    return operator new[](size);
}

inline std::size_t EnsureValidSize(std::size_t size) {
    return (size == 0) ? 1 : size;
}

void* operator new(std::size_t size, std::align_val_t al)
{
    size = EnsureValidSize(size);
    std::size_t alignment = static_cast<std::size_t>(al);
    
    return based::MemoryManager::MemAlign(size, alignment);
}

void* operator new[](std::size_t size, std::align_val_t al)
{
    size = EnsureValidSize(size);
    std::size_t alignment = static_cast<std::size_t>(al);
    
    return based::MemoryManager::MemAlign(size, alignment);
}

void* operator new(std::size_t size, std::align_val_t al, const std::nothrow_t&) noexcept
{
    return operator new(size, al);
}

void* operator new[](std::size_t size, std::align_val_t al, const std::nothrow_t&) noexcept
{
    return operator new[](size, al);
}

void operator delete(void* ptr) noexcept
{
    based::MemoryManager::MemFree(ptr);
}
// Don't need a delete[] overload because it wouldn't do anything delete doesn't already do

void*  my_malloc(size_t size)
{
    return based::MemoryManager::MemAlign(size, based::GetPlatformDefaultAlignment());
}

void   my_free(void* ptr)
{
    return based::MemoryManager::MemFree(ptr);
}

void*  my_realloc(void* ptr, size_t size)
{
    return based::MemoryManager::MemRealloc(ptr, size);
}

void*  my_calloc(size_t num, size_t size)
{
    void* ptr = based::MemoryManager::MemAlign(num * size, based::GetPlatformDefaultAlignment());
    if (ptr)
        memset(ptr, 0, num * size);
    return ptr;
}

// C11 aligned allocation
BASED_C_ALLOC_OVERRIDE void*  aligned_alloc(size_t alignment, size_t size)
{
    return based::MemoryManager::MemAlign(size, alignment);
}

// POSIX aligned allocation
#ifdef BASED_PLATFORM_LINUX
void*  malloc(size_t size)
{
    return my_malloc(size);
}

void   free(void* ptr)
{
    return my_free(ptr);
}

void*  realloc(void* ptr, size_t size)
{
    return return my_realloc(ptr, size);
}

void*  calloc(size_t num, size_t size)
{
    return my_calloc(num, size);
}

int    posix_memalign(void** ptr, size_t alignment, size_t size)
{
    if (stAlignment < sizeof(void*) || (stAlignment & (stAlignment - 1)) != 0)
        return EINVAL;
    
    *ptr = based::MemoryManager::MemAlign(size, alignment);
    if (!*ptr)
        return ENOMEM;

    return 0;
}
#endif
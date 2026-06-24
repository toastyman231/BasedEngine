#pragma once

#include <new>
#include <cstdlib>

#include "../memory/MemoryManager.h"
#include "../memory/PlatformMemUtils.h"

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
#pragma once

#include <vfspp/VirtualFileSystem.hpp>
#include <vfspp/NativeFileSystem.hpp>
#include <vfspp/MemoryFileSystem.hpp>

#if defined(_WIN32)
#include <windows.h>
#elif defined(__linux__)
#include <unistd.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#endif

namespace based
{
    inline vfspp::VirtualFileSystemPtr FileSystem;

    bool InitializeRootFileSystem();

    std::string GetExecutableDirectory();
}

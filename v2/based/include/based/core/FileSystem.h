#pragma once

#include <vfspp/VirtualFileSystem.hpp>
#include <vfspp/NativeFileSystem.hpp>
#include <vfspp/MemoryFileSystem.hpp>

namespace based
{
    inline vfspp::VirtualFileSystemPtr FileSystem;

    bool InitializeRootFileSystem();

    std::string GetExecutableDirectory();
}

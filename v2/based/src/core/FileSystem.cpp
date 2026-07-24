#include "pch.h"
#include "core/FileSystem.h"

#include "core/BasedLog.h"

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#elif defined(__linux__)
#include <unistd.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#endif

static vfspp::NativeFileSystemPtr g_pRootFs;

namespace based
{
    bool InitializeRootFileSystem()
    {
        static bool bDoOnce = false;
        BASED_ASSERT(!bDoOnce, "Trying to initialize the file system a second time!");
        
        FileSystem = std::make_shared<vfspp::VirtualFileSystem>();

        // TODO: Decide whether resource bundling should be release only or also in debug/development
        auto maybeFs =
            FileSystem->CreateFileSystem<vfspp::NativeFileSystem>("/", GetExecutableDirectory());
        if (maybeFs.has_value())
        {
            g_pRootFs = maybeFs.value();
            bDoOnce = true;
            return true;
        }

        BASED_ERROR("Error creating native filesystem!");
        return false;
    }

    std::string GetExecutableDirectory()
    {
#if defined(_WIN32)
        wchar_t buffer[MAX_PATH];
        GetModuleFileNameW(NULL, buffer, MAX_PATH);
        return std::filesystem::path(buffer).parent_path().string();

#elif defined(__linux__)
        return std::filesystem::read_symlink("/proc/self/exe").parent_path().string();

#elif defined(__APPLE__)
        uint32_t size = 0;
        _NSGetExecutablePath(nullptr, &size);
        std::vector<char> buffer(size);
        if (_NSGetExecutablePath(buffer.data(), &size) == 0) {
            return std::filesystem::canonical(buffer.data()).parent_path().string();
        }
        return "";
#elif defined(__EMSCRIPTEN__)
        mkdir("/GameDir", 0777);
        return "/GameDir";
#else
#error "Unsupported platform! Implement this function for the current platform!"
        return ""; // Unsupported platform
#endif
    }
}

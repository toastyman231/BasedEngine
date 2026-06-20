project "mimalloc"
    kind "SharedLib"
    language "C"
    location "Intermediate"
    targetdir "../../../PostBuildCopy/%{cfg.system}"
    implibdir "lib/Windows"
    objdir "bin-obj/%{cfg.buildcfg}/%{prj.name}"

    files
    {
        "include/mimalloc.h",
        "include/mimalloc-override.h",
        "include/mimalloc-new-delete.h",
        "include/mimalloc-stats.h",
        "src/alloc.c",
        "src/alloc-aligned.c",
        "src/alloc-posix.c",
        "src/arena.c",
        "src/arena-meta.c",
        "src/bitmap.c",
        "src/heap.c",
        "src/init.c",
        "src/libc.c",
        "src/options.c",
        "src/os.c",
        "src/page.c",
        "src/page-map.c",
        "src/random.c",
        "src/stats.c",
        "src/theap.c",
        "src/threadlocal.c",
        "src/prim/prim.c",
    }

    includedirs
    {
        "include",
    }

    defines
    {
        "MI_SHARED_LIB",
        "MI_SHARED_LIB_EXPORT",
        "MI_MALLOC_OVERRIDE",
    }

    enablepch "off"

    filter "system:windows"
        systemversion "latest"
        -- mimalloc requires C++ mode on MSVC for modern atomics
        language "C++"
        cppdialect "C++17"
        staticruntime "off" -- MUST BE OFF FOR MIMALLOC-REDIRECT TO WORK!
        defines
        {
            "_WIN32_WINNT=0x600", -- this is just for MinGW compatibility, but it's harmless so we keep it for all compilers
            "_WINDLL",
        }
        links
        {
            "psapi", "shell32", "user32", "advapi32", "bcrypt", "mimalloc-redirect"
        }
        libdirs { "lib/%{cfg.system}"}

    filter "system:linux"
        pic "On"
        systemversion "latest"
        links { "pthread", "rt" }

    filter "system:macosx"
        pic "On"
        systemversion "latest"
        links { "pthread" }
        defines
        {
            "MI_OSX_INTERPOSE=1",
            "MI_OSX_ZONE=1",
        }

    filter "configurations:Debug*"
        runtime "Debug"
        symbols "on"
        defines { "MI_DEBUG=2" }
        -- No targetsuffix for mimalloc! It messed with it's ability to link mimalloc-redirect I think

    filter "configurations:Development*"
        runtime "Release"
        symbols "on"
        optimize "on"
        defines { "MI_DEBUG=2" }
        -- No targetsuffix for mimalloc! It messed with it's ability to link mimalloc-redirect I think

    filter "configurations:Release*"
        runtime "Release"
        optimize "on"
        defines { "MI_BUILD_RELEASE" }
    
    filter {}
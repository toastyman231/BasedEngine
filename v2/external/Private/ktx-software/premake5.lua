project "ktx-software"
    kind "StaticLib"
    language "C++"
    cppdialect "C++11"
    location "Intermediate"
    targetdir "bin/%{cfg.buildcfg}/%{prj.name}"
    objdir "bin-obj/%{cfg.buildcfg}/%{prj.name}"

    files
    {
        "include/ktx.h",
        "include/KHR/khr_df.h",
        "lib/**.h",
        "lib/**.c",
        "lib/**.cpp",
        "lib/**.cxx",
        "external/dfdutils/*.c",
        "external/dfdutils/*.h",
        "external/basisu/transcoder/basisu_transcoder.cpp",
        "external/basisu/transcoder/basisu_transcoder.h",
        "external/basisu/zstd/zstd.c",
    }

    removefiles
    {
        -- Write/encode support not needed for loading
        "lib/basis_encode.cpp",
        "lib/writer1.c",
        "lib/writer2.c",
        -- GL upload loader (we are using Vulkan and D3D12)
        "lib/gl_funcs.c",
        "lib/glloader.c",
        -- Not needed on x64
        "external/dfdutils/endswap.c",
    }

    includedirs
    {
        "include",
        "include/KHR",
        "utils",
        "lib",
        "external",
        "other_include",
        "external/basisu/transcoder",
        "external/basisu/zstd",
        "external/dfdutils",
        "external/dfdutils/vulkan",
    }

    defines
    {
        "LIBKTX",
        "KHRONOS_STATIC",
        "KTX_FEATURE_KTX1",
        "KTX_FEATURE_KTX2",
        "SUPPORT_SOFTWARE_ETC_UNPACK=1",
        "BASISD_SUPPORT_FXT1=0",
        "BASISD_SUPPORT_KTX2=0",
        "BASISD_SUPPORT_KTX2_ZSTD=0",
    }

    enablepch "off"

    filter "system:windows"
        systemversion "latest"
        staticruntime "off" -- MUST BE OFF FOR MIMALLOC-REDIRECT TO WORK!
        defines
        {
            "_SCL_SECURE_NO_WARNINGS",
        }

    filter "system:linux"
        pic "On"
        systemversion "latest"
        links { "dl", "pthread" }

    filter "system:macosx"
        pic "On"
        systemversion "latest"

    filter "configurations:Debug*"
        runtime "Debug"
        symbols "on"
        defines { "_DEBUG", "DEBUG" }
        targetsuffix "_d"

    filter "configurations:Development*"
        runtime "Release"
        symbols "on"
        optimize "on"
        defines { "_DEBUG", "DEBUG" }
        targetsuffix "_dev"

    filter "configurations:Release*"
        runtime "Release"
        optimize "on"
project "based"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"
    location "../Intermediate"
    targetdir "bin/%{cfg.buildcfg}/%{prj.name}"
    objdir "bin-obj/%{cfg.buildcfg}/%{prj.name}"

    pchheader "pch.h"
    pchsource "src/pch.cpp"

    EXTERNALS_DIR_PRIVATE = "../external/Private/"
    EXTERNALS_DIR_PUBLIC = "../external/Public/"

    files 
    {
        "include/**.h",
        "include/**.hpp",
        "src/**.h",
        "src/**.cpp",
        "**.natvis"
    }

    includedirs
    {
        EXTERNALS_DIR_PUBLIC,
        "include/based",
        EXTERNALS_DIR_PRIVATE .. "assimp/include",
        EXTERNALS_DIR_PRIVATE .. "freetype/include",
        EXTERNALS_DIR_PRIVATE .. "jolt/include",
        EXTERNALS_DIR_PRIVATE .. "ktx-software/include",
        EXTERNALS_DIR_PRIVATE .. "mimalloc/include",
        EXTERNALS_DIR_PRIVATE .. "rmlui/include",
        EXTERNALS_DIR_PRIVATE .. "sdl3/include",
        EXTERNALS_DIR_PRIVATE .. "spdlog/include",
        EXTERNALS_DIR_PRIVATE .. "tracy/public",
        EXTERNALS_DIR_PRIVATE .. "yaml-cpp/include",
    }

    defines
    {
        "_CRT_SECURE_NO_WARNINGS",
        "_SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING",
        "RMLUI_STATIC_LIB",
        "KHRONOS_STATIC",
        "GLM_ENABLE_EXPERIMENTAL",
        "MI_SHARED_LIB"
    }

    links
    {
        "assimp",
        "freetype",
        "jolt",
        "ktx-software",
        "mimalloc",
        "rmlui",
        "sdl3",
        "spdlog",
        "tracy",
        "yaml-cpp"
    }  

    libdirs
    {
        EXTERNALS_DIR_PRIVATE .. "mimalloc/lib/%{cfg.system}"
    }

    filter {"system:windows", "configurations:*"}
        systemversion "latest"
        staticruntime "off" -- MUST BE OFF FOR MIMALLOC-REDIRECT TO WORK!
        defines { "BASED_PLATFORM_WINDOWS" }
        buildoptions "/bigobj"

    filter {"system:macosx", "configurations:*"}
        xcodebuildsettings
        {
            ["MACOSX_DEPLOYMENT_TARGET"] = "10.15",
            ["UseModernBuildSystem"] = "NO"
        }

        defines "BASED_PLATFORM_MAC"

    filter {"system:linux", "configurations:*"}
        defines "BASED_PLATFORM_LINUX"

    filter "configurations:Debug*"
        defines 
        {
            "BASED_CONFIG_DEBUG",
            "JPH_DEBUG_RENDERER"
        }
        runtime "Debug"
        editandcontinue "off"
        symbols "on"
        targetsuffix "_d"

    filter "configurations:Development*"
        defines 
        {
            "BASED_CONFIG_DEVELOPMENT",
            "JPH_DEBUG_RENDERER"
        }
        runtime "Release"
        editandcontinue "off"
        optimize "on"
        symbols "on"
        targetsuffix "_dev"

    filter "configurations:Release*"
        defines "BASED_CONFIG_RELEASE"
        fatalwarnings "All"
        runtime "Release"
        symbols "off"
        optimize "on"

    filter "configurations:*Editor"
        defines "BASED_CONFIG_EDITOR"
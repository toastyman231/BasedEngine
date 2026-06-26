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
        "**.natvis",
    }

    -- Not all of these are actually supported
    local systems = {
        "Windows",
        "Linux",
        "MacOSX",
        "PS5",
        "Xbox",
        "Android",
        "iOS"
    }

    -- Remove any platform specific files, we will add back just the ones for this platform later
    for _, sys in ipairs(systems) do
        removefiles 
        { 
            "include/**/" .. sys .. "/**.h",
            "include/**/" .. sys .. "/**.hpp",
            "src/**/" .. sys .. "/**.c",
            "src/**/" .. sys .. "/**.cpp",
        }
    end

    files
    {
        "include/**/%{cfg.system}/**.h",
        "include/**/%{cfg.system}/**.hpp",
        "src/**/%{cfg.system}/**.c",
        "src/**/%{cfg.system}/**.cpp",
    }

    includedirs
    {
        EXTERNALS_DIR_PUBLIC,
        "include/based",
        EXTERNALS_DIR_PRIVATE .. "assimp/include",
        EXTERNALS_DIR_PRIVATE .. "freetype/include",
        EXTERNALS_DIR_PRIVATE .. "jolt/include",
        EXTERNALS_DIR_PRIVATE .. "ktx-software/include",
        EXTERNALS_DIR_PRIVATE .. "rmlui/include",
        EXTERNALS_DIR_PRIVATE .. "sdl3/include",
        EXTERNALS_DIR_PRIVATE .. "spdlog/include",
        EXTERNALS_DIR_PRIVATE .. "tlsf/include",
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
    }

    links
    {
        "assimp",
        "freetype",
        "jolt",
        "ktx-software",
        "rmlui",
        "sdl3",
        "spdlog",
        "tlsf",
        "tracy",
        "yaml-cpp"
    }  

    filter {"system:windows", "configurations:*"}
        systemversion "latest"
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
        optimize "debug"
        targetsuffix "_d"

    filter "configurations:Development*"
        defines 
        {
            "BASED_CONFIG_DEBUG",
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
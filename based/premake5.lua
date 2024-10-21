if not importedDependencies then include "../dependencies.lua" end

project "based"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"
    staticruntime "on"

    targetdir(tdir)
    objdir(odir)

    pchheader "pch.h"
    pchsource "src/pch.cpp"

    files 
    {
        "include/**.h",
        "include/**.hpp",
        "include/**.cpp",
        "src/**.h",
        "src/**.cpp",
        "**.natvis"
    }

    includedirs
    {
        "include",
        "include/based",
        "include/based/core",
        "%{externals.sdl2}",
        "%{externals.spdlog}",
        "%{externals.glad}",
        "%{externals.freetype}",
        "%{externals.miniaudio}",
        "%{externals.assimp}",
        "%{externals.rmlui}",
        "%{externals.tracy}",
        "%{externals.yaml_cpp}",
        --"C:/Program Files (x86)/Visual Leak Detector/include"
    }

    defines
    {
        "GLFW_INCLUDE_NONE", -- Ensures glad doesn't include glfw
        "_CRT_SECURE_NO_WARNINGS",
        "_SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING"
    }

    links
    {
        "SDL2",
        "SDL2_ttf",
        "glad",
        "freetype",
        "assimp-vc143-mt",
        "RmlCore",
        "RmlDebugger",
        "yaml-cpp",
        --"vld"
    }

    libdirs
    {
        "%{libraries.sdl2}",
        "%{libraries.spdlog}",
        "%{libraries.freetype}",
        "%{libraries.miniaudio}",
        "%{libraries.assimp}",
        "%{libraries.rmlui}",
        "%{libraries.tracy}",
        --"C:/Program Files (x86)/Visual Leak Detector/lib"
    }

    filter {"system:windows", "configurations:*"}
        systemversion "latest"
        defines "BASED_PLATFORM_WINDOWS"

    filter {"system:macosx", "configurations:*"}
        xcodebuildsettings
        {
            ["MACOSX_DEPLOYMENT_TARGET"] = "10.15",
            ["UseModernBuildSystem"] = "NO"
        }

        defines "BASED_PLATFORM_MAC"

    filter {"system:linux", "configurations:*"}
        defines "BASED_PLATFORM_LINUX"

    filter "configurations:Debug"
        defines "BASED_CONFIG_DEBUG"
        runtime "Debug"
        editandcontinue "off"
        symbols "on"

    filter "configurations:Release"
        defines "BASED_CONFIG_RELEASE"
        flags "FatalWarnings"
        runtime "Release"
        symbols "off"
        optimize "on"
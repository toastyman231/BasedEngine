include "D:\\Jake\\Documents\\Github_Repos\\BasedEngine"
-- Process Glad before anything else
include "D:\\Jake\\Documents\\Github_Repos\\BasedEngine\\external\\glad"

workspace "BasedTetris"
    startproject "BasedTetris"
    architecture "x64"

    configurations
    {
        "Debug",
        "Release"
    }

project "BasedTetris"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    staticruntime "on"
    links "based"

    targetdir(tdir)
    objdir(odir)

    files 
    {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp"
    }

    externalincludedirs
    {
        "D:\\Jake\\Documents\\Github_Repos\\BasedEngine\\based\\include",
        "D:\\Jake\\Documents\\Github_Repos\\BasedEngine\\%{externals.sdl2}\\include",
        "D:\\Jake\\Documents\\Github_Repos\\BasedEngine\\%{externals.spdlog}\\include"
    }

    flags
    {
        "FatalWarnings"
    }

    postbuildcommands
    {
        "python3 " .. string.sub(path.getabsolute("%{prj.name}"), 1, string.len(path.getabsolute("%{prj.name}")) - (string.len("%{prj.name}") + 1)) .. "/postbuild.py config=%{cfg.buildcfg} prj=%{prj.name}"
    }   

    filter {"system:windows", "configurations:*"}
        systemversion "latest"

        defines
        {
            "BASED_PLATFORM_WINDOWS"
        }

        libdirs
        {
            "D:\\Jake\\Documents\\Github_Repos\\BasedEngine\\bin\\Debug\\based",
            "D:\\Jake\\Documents\\Github_Repos\\BasedEngine\\external\\glad\\bin\\Debug\\glad",
            "D:\\Jake\\Documents\\Github_Repos\\BasedEngine\\%{externals.sdl2}\\lib",
            "D:\\Jake\\Documents\\Github_Repos\\BasedEngine\\%{externals.freetype}\\lib"
        }

        links
        {
            "SDL2",
            "SDL2_ttf",
            "glad",
            "freetype"
        }

    filter {"system:macosx", "configurations:*"}
        xcodebuildsettings
        {
            ["MACOSX_DEPLOYMENT_TARGET"] = "10.15",
            ["UseModernBuildSystem"] = "NO"
        }

        defines
        {
            "BASED_PLATFORM_MAC"
        }

        abspath = path.getabsolute("%{externals.maclibs}")
        linkoptions {"-F " .. abspath}

        links
        {
            "SDL2.framework",
            "glad",
            "freetype"
        }

    filter {"system:linux", "configurations:*"}
        defines
        {
            "BASED_PLATFORM_LINUX"
        }

        links
        {
            "SDL2",
            "glad",
            "dl",
            "freetype"
        }

    filter "configurations:Debug"
        defines
        {
            "BASED_CONFIG_DEBUG"
        }
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        defines
        {
            "BASED_CONFIG_RELEASE"
        }
        runtime "Release"
        symbols "off"
        optimize "on"
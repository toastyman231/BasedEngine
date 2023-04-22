include "C:\\Users\\jmorg\\Documents\\Repos\\BasedEngine"
-- Process Glad before anything else
include "C:\\Users\\jmorg\\Documents\\Repos\\BasedEngine\\external\\glad"

workspace "Sandbox"
    startproject "Sandbox"
    architecture "x64"

    configurations
    {
        "Debug",
        "Release"
    }

project "Sandbox"
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
        "C:\\Users\\jmorg\\Documents\\Repos\\BasedEngine\\based\\include",
        "C:\\Users\\jmorg\\Documents\\Repos\\BasedEngine\\%{externals.sdl2}\\include",
        "C:\\Users\\jmorg\\Documents\\Repos\\BasedEngine\\%{externals.spdlog}\\include"
    }

    flags
    {
        "FatalWarnings"
    }

    postbuildcommands
    {
        "python3 " .. string.sub(path.getabsolute("%{prj.name}"), 1, string.len(path.getabsolute("%{prj.name}")) - (string.len("%{prj.name}") + 1)) .. "/postbuild.py config=%{cfg.buildcfg} prj=%{prj.name}"
    }   

    filter {"system:windows", "configurations:Debug"}
        systemversion "latest"

        defines
        {
            "BASED_PLATFORM_WINDOWS"
        }

        libdirs
        {
            "C:\\Users\\jmorg\\Documents\\Repos\\BasedEngine\\bin\\Debug\\based",
            "C:\\Users\\jmorg\\Documents\\Repos\\BasedEngine\\external\\glad\\bin\\Debug\\glad",
            "C:\\Users\\jmorg\\Documents\\Repos\\BasedEngine\\%{externals.sdl2}\\lib",
            "C:\\Users\\jmorg\\Documents\\Repos\\BasedEngine\\%{externals.freetype}\\lib"
        }

        links
        {
            "SDL2",
            "SDL2_ttf",
            "glad",
            "freetype"
        }

    filter {"system:windows", "configurations:Release"}
        systemversion "latest"

        defines
        {
            "BASED_PLATFORM_WINDOWS"
        }

        libdirs
        {
            "C:\\Users\\jmorg\\Documents\\Repos\\BasedEngine\\bin\\Release\\based",
            "C:\\Users\\jmorg\\Documents\\Repos\\BasedEngine\\external\\glad\\bin\\Release\\glad",
            "C:\\Users\\jmorg\\Documents\\Repos\\BasedEngine\\%{externals.sdl2}\\lib",
            "C:\\Users\\jmorg\\Documents\\Repos\\BasedEngine\\%{externals.freetype}\\lib"
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
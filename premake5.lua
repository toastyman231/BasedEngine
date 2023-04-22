workspace "based"
    startproject "basededitor"
    architecture "x64"

    configurations
    {
        "Debug",
        "Release"
    }

tdir = "bin/%{cfg.buildcfg}/%{prj.name}"
odir = "bin-obj/%{cfg.buildcfg}/%{prj.name}"

-- External Dependencies
externals = {}
externals["maclibs"] = "external/maclibs"
externals["sdl2"] = "external/sdl2"
externals["spdlog"] = "external/spdlog"
externals["glad"] = "external/glad"
externals["freetype"] = "external/freetype"
externals["miniaudio"] = "external/miniaudio"
externals["assimp"] = "external/assimp"

-- Process Glad before anything else
include "external/glad"

project "based"
    location "based"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"
    staticruntime "on"

    targetdir(tdir)
    objdir(odir)

    files 
    {
        "%{prj.name}/include/**.h",
        "%{prj.name}/include/**.hpp",
        "%{prj.name}/include/**.cpp",
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp",
        "%{prj.name}/**.natvis"
    }

    externalincludedirs
    {
        "%{prj.name}/include",
        "%{prj.name}/include/based",
        "%{prj.name}/include/based/core",
        "%{externals.sdl2}/include",
        "%{externals.spdlog}/include",
        "%{externals.glad}/include",
        "%{externals.freetype}/include",
        "%{externals.miniaudio}",
        "%{externals.assimp}/include"
    }

    flags
    {
        "FatalWarnings"
    }

    defines
    {
        "GLFW_INCLUDE_NONE", -- Ensures glad doesn't include glfw
        "_CRT_SECURE_NO_WARNINGS"
    }

    filter {"system:windows", "configurations:*"}
        systemversion "latest"

        defines
        {
            "BASED_PLATFORM_WINDOWS"
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

    filter {"system:linux", "configurations:*"}
        defines
        {
            "BASED_PLATFORM_LINUX"
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

project "basededitor"
    location "basededitor"
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
        "based/include",
        "%{externals.spdlog}/include"
    }

    flags
    {
        "FatalWarnings"
    }

    postbuildcommands
    {
        "python3 " .. path.getabsolute("%{prj.name}") .. "/postbuild.py  config=%{cfg.buildcfg} prj=%{prj.name}"
    }

    filter {"system:windows", "configurations:*"}
        systemversion "latest"

        defines
        {
            "BASED_PLATFORM_WINDOWS"
        }

        libdirs
        {
            "%{externals.sdl2}/lib",
            "%{externals.freetype}/lib",
            "%{externals.assimp}/lib"
        }

        links
        {
            "SDL2",
            "SDL2_ttf",
            "glad",
            "freetype",
            "assimp-vc143-mt"
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
            "SDL2_ttf",
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

project "PongV1"
        location "PongV1"
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
            "based/include",
            "%{externals.spdlog}/include"
        }
    
        flags
        {
            "FatalWarnings"
        }

        postbuildcommands
        {
            "python3 " .. path.getabsolute("%{prj.name}") .. "/postbuild.py config=%{cfg.buildcfg} prj=%{prj.name}"
        }   
    
        filter {"system:windows", "configurations:*"}
        systemversion "latest"

        defines
        {
            "BASED_PLATFORM_WINDOWS"
        }

        libdirs
        {
            "%{externals.sdl2}/lib",
            "%{externals.freetype}/lib"
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

project "Sandbox"
    location "Sandbox"
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
        "based/include",
        "%{externals.sdl2}/include",
        "%{externals.spdlog}/include",
        "%{externals.miniaudio}",
        "%{externals.assimp}/include"
    }

    flags
    {
        "FatalWarnings"
    }

    postbuildcommands
    {
        "python3 " .. path.getabsolute("%{prj.name}") .. "/postbuild.py config=%{cfg.buildcfg} prj=%{prj.name}"
    }   

    filter {"system:windows", "configurations:*"}
        systemversion "latest"

        defines
        {
            "BASED_PLATFORM_WINDOWS"
        }

        libdirs
        {
            "%{externals.sdl2}/lib",
            "%{externals.freetype}/lib",
            "%{externals.assimp}/lib"
        }

        links
        {
            "SDL2",
            "SDL2_ttf",
            "glad",
            "freetype",
            "assimp-vc143-mt"
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
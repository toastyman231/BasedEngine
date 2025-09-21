engineLocation = os.getenv("BASED_ENGINE_HOME")
if not engineLocation then
    error("BASED_ENGINE_HOME environment variable is not set. Please try restarting your terminal, or configure it to point to the engine directory.")
end

project "Sandbox"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    staticruntime "on"

    targetdir(tdir)
    objdir(odir)

    links
    {
        "based",
        "SDL2",
        "freetype",
        "assimp-vc143-mt",
        "rmlui",
        "rmlui_debugger",
        "ktx",
    }

    files 
    {
        "src/**.h",
        "src/**.cpp"
    }

    includedirs
    {
        "%{wks.location}/based/include",
        "%{externals.spdlog}",
        "%{externals.rmlui}",
        "%{externals.tracy}",
        "%{externals.yaml_cpp}",
        "%{externals.jolt}"
    }

    libdirs
    {
        "%{libraries.sdl2}",
        "%{libraries.freetype}",
        "%{libraries.assimp}",
        "%{libraries.rmlui}",
        "%{libraries.ktx_software}"
    }

    include "Plugins/FMOD"

    flags
    {
        "FatalWarnings"
    }

    postbuildcommands
    {
        engineLocation .. "/tools/bin/BasedBuildTool -i " .. path.getabsolute("../%{prj.name}") .. " -gen-mipmaps -c %{cfg.buildcfg}" 
    }

    filter {"system:windows", "configurations:*"}
        systemversion "latest"
        files { "resources.rc", "Assets/**.ico" }
        vpaths { ['Assets/*'] = { '*.rc', '**.ico' } }
        debugdir(tdir)

        defines
        {
            "BASED_PLATFORM_WINDOWS",
            "_SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING",
            "JPH_DEBUG_RENDERER"
        }

    filter {"system:windows", "configurations:Release"}
        kind "WindowedApp"
    filter {"system:macosx", "configurations:Release"}
        kind "WindowedApp"

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
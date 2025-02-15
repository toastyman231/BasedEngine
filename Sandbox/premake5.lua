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

    include "Plugins/FMOD"

    flags
    {
        "FatalWarnings"
    }

    postbuildcommands
    {
        "python3 " .. path.getabsolute("../%{prj.name}") .. "/postbuild.py config=%{cfg.buildcfg} prj=%{prj.name}"
    }

    filter {"system:windows", "configurations:*"}
        systemversion "latest"
        files { "resources.rc", "Assets/**.ico" }
        vpaths { ['Assets/*'] = { '*.rc', '**.ico' } }

        defines
        {
            "BASED_PLATFORM_WINDOWS",
            "_SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING",
            "JPH_DEBUG_RENDERER"
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
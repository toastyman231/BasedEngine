project "BasedEditor"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    staticruntime "on"

    if externalBuild then location "%{wks.location}/ProjectFiles" end

    if externalBuild then targetdir("%{wks.location}/ProjectFiles/" .. tdir) else targetdir(tdir) end
    if externalBuild then objdir("%{wks.location}/ProjectFiles/" .. odir) else objdir(odir) end

    files 
    {
        "src/**.h",
        "src/**.cpp"
    }

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

    includedirs(
        (externalBuild and {
            (engineLocation .. "//based//include"),
            (engineLocation .. "%{externals.spdlog}"),
            (engineLocation .. "%{externals.rmlui}"),
            (engineLocation .. "%{externals.tracy}"),
            (engineLocation .. "%{externals.yaml_cpp}"),
            (engineLocation .. "%{externals.jolt}")
        } or {
            "%{wks.location}/based/include",
            "%{externals.spdlog}",
            "%{externals.rmlui}",
            "%{externals.tracy}",
            "%{externals.yaml_cpp}",
            "%{externals.jolt}",
            "%{externals.assimp}",
        }
    ))

    libdirs(
        (externalBuild and {
            (engineLocation .. "%{libraries.sdl2}"),
            (engineLocation .. "%{libraries.freetype}"),
            (engineLocation .. "%{libraries.assimp}"),
            (engineLocation .. "%{libraries.rmlui}"),
            (engineLocation .. "%{libraries.ktx_software}"),
        } or {
            "%{libraries.sdl2}",
            "%{libraries.freetype}",
            "%{libraries.assimp}",
            "%{libraries.rmlui}",
            "%{libraries.ktx_software}"
        }
    ))

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

        if externalBuild then debugargs({ "%{wks.location}/" }) end
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
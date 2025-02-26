project "BasedEditor"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    staticruntime "on"
    links "based"

    if externalBuild then location "%{wks.location}/ProjectFiles" end

    if externalBuild then targetdir("%{wks.location}/ProjectFiles/" .. tdir) else targetdir(tdir) end
    if externalBuild then objdir("%{wks.location}/ProjectFiles/" .. odir) else objdir(odir) end

    files 
    {
        "src/**.h",
        "src/**.cpp"
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
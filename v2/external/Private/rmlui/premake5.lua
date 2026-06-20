project "rmlui"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"
    location "Intermediate"
    targetdir "bin/%{cfg.buildcfg}/%{prj.name}"
    objdir "bin-obj/%{cfg.buildcfg}/%{prj.name}"

    files
    {
        "Include/**.h",
        "Include/**.hpp",
        "Source/Core/**.h",
        "Source/Core/**.cpp",
        "Source/Debugger/**.h",
        "Source/Debugger/**.cpp",
    }

    includedirs
    {
        "Include",
        "../freetype/include",
    }

    links
    {
        "freetype",
    }

    defines
    {
        "RMLUI_STATIC_LIB",
    }

    enablepch "off"

    filter "system:windows"
        systemversion "latest"
        staticruntime "off" -- MUST BE OFF FOR MIMALLOC-REDIRECT TO WORK!
        defines
        {
            "_CRT_SECURE_NO_WARNINGS",
            "WIN32_LEAN_AND_MEAN",
            "NOMINMAX",
        }

    filter "system:linux"
        pic "On"
        systemversion "latest"

    filter "system:macosx"
        pic "On"
        systemversion "latest"

    filter "configurations:Debug*"
        runtime "Debug"
        symbols "on"
        targetsuffix "_d"

    filter "configurations:Development*"
        runtime "Release"
        symbols "on"
        optimize "on"
        targetsuffix "_dev"

    filter "configurations:Release*"
        runtime "Release"
        optimize "on"
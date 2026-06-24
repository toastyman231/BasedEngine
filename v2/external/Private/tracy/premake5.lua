project "tracy"
    kind "StaticLib"
    language "C++"
    cppdialect "C++11"
    location "Intermediate"
    targetdir "bin/%{cfg.buildcfg}/%{prj.name}"
    objdir "bin-obj/%{cfg.buildcfg}/%{prj.name}"

    files
    {
        "public/**.hpp",
        "public/**.h",
        "public/TracyClient.cpp",
    }

    includedirs
    {
        "public"
    }

    enablepch "off"

    filter "system:windows"
        systemversion "latest"
        links { "ws2_32", "dbghelp" }
        linkoptions { "/ignore:4006" }

    filter "system:linux"
        pic "On"
        systemversion "latest"
        links { "dl", "pthread" }

    filter "system:macosx"
        pic "On"
        systemversion "latest"
        links { "pthread" }

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
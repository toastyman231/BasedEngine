project "tracy"
    kind "StaticLib"
    language "C++"
    cppdialect "C++11"
    location "Intermediate"
    targetdir(tdir)
    objdir(odir)

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
    filter {}

    filter "system:linux"
        pic "On"
        systemversion "latest"
        links { "dl", "pthread" }
    filter {}

    filter "system:macosx"
        pic "On"
        systemversion "latest"
        links { "pthread" }
    filter {}

    filter "configurations:Debug*"
        runtime "Debug"
        symbols "on"
        targetsuffix "_d"
    filter {}

    filter "configurations:Development*"
        runtime "Release"
        symbols "on"
        optimize "on"
        targetsuffix "_dev"
    filter {}

    filter "configurations:Release*"
        runtime "Release"
        optimize "on"
    filter {}
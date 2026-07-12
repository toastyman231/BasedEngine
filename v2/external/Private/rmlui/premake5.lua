project "rmlui"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"
    location "Intermediate"
    targetdir(tdir)
    objdir(odir)

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
        defines
        {
            "_CRT_SECURE_NO_WARNINGS",
            "WIN32_LEAN_AND_MEAN",
            "NOMINMAX",
        }
    filter {}

    filter "system:linux or macosx"
        pic "On"
        systemversion "latest"
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
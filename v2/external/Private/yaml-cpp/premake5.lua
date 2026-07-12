project "yaml-cpp"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"
    location "Intermediate"
    targetdir(tdir)
    objdir(odir)

    files 
    {
        "include/**.h",
        "src/**.h",
        "src/**.cpp"
    }

    includedirs
    {
        "include"
    }

    enablepch "Off"

    defines 
    {
        "YAML_CPP_STATIC_DEFINE"
    }

    filter "system:windows"
        systemversion "latest"
        links { "ws2_32", "dbghelp" }
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
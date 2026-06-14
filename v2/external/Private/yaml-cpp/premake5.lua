project "yaml-cpp"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"
    staticruntime "on"
    location "Intermediate"
    targetdir "bin/%{cfg.buildcfg}/%{prj.name}"
    objdir "bin-obj/%{cfg.buildcfg}/%{prj.name}"

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
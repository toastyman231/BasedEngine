project "spdlog"
    kind "StaticLib"
    language "C++"
    cppdialect "C++11"
    location "Intermediate"
    targetdir "bin/%{cfg.buildcfg}/%{prj.name}"
    objdir "bin-obj/%{cfg.buildcfg}/%{prj.name}"

    files
    {
        "include/**.h",
        "src/**.cpp"
    }

    includedirs
    {
        "include"
    }

    defines
    {
        "SPDLOG_COMPILED_LIB",
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
        buildoptions { "/utf-8", "/Zc:__cplusplus" }

    filter "system:linux"
        pic "On"
        systemversion "latest"
        links { "pthread" }

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
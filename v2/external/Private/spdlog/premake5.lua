project "spdlog"
    kind "StaticLib"
    language "C++"
    cppdialect "C++11"
    location "Intermediate"
    targetdir(tdir)
    objdir(odir)

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

    --[[ if os.host() == "windows" then
        buildoptions { "/utf-8", "/Zc:__cplusplus" }
    end ]]

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
        links { "pthread" }
    filter {}

    filter "system:emscripten"
        pic "On"
        systemversion "latest"
        linkoptions  { "-pthread" }
        buildoptions { "-pthread" }
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
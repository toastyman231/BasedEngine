project "yaml-cpp"
    kind "StaticLib"
    language "C++"

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

    flags "NoPCH"

    defines 
    {
        "YAML_CPP_STATIC_DEFINE"
    }

    filter {"system:windows"}
        systemversion "latest"
        cppdialect "C++17"
        staticruntime "off"

    filter {"system:macosx"}
        xcodebuildsettings
        {
            ["MACOSX_DEPLOYMENT_TARGET"] = "10.15",
            ["UseModernBuildSystem"] = "NO"
        }

    filter "configurations:Debug"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        runtime "Release"
        symbols "off"
        optimize "on"
project "jolt"
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
        "include/**.cpp"
    }

    includedirs
    {
        "include"
    }
    
    enablepch "off"
    
    filter "system:windows"
		systemversion "latest"

	filter "system:linux"
		pic "On"
		systemversion "latest"

    filter "configurations:Debug*"
        runtime "Debug"
        symbols "on"
        targetsuffix "_d"
        defines
        {
            "JPH_DEBUG_RENDERER"
        }

    filter "configurations:Development*"
        runtime "Release"
        symbols "on"
        optimize "on"
        targetsuffix "_dev"
        defines
        {
            "JPH_DEBUG_RENDERER"
        }

    filter "configurations:Release*"
        runtime "Release"
        optimize "on"
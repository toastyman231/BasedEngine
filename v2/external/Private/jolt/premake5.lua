project "jolt"
    kind "StaticLib"
    language "C++"
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
		cppdialect "C++17"
        staticruntime "off" -- MUST BE OFF FOR MIMALLOC-REDIRECT TO WORK!

	filter "system:linux"
		pic "On"
		systemversion "latest"
		cppdialect "C++17"

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"
        defines
        {
            "JPH_DEBUG_RENDERER"
        }

	filter "configurations:Release"
		runtime "Release"
		optimize "on"
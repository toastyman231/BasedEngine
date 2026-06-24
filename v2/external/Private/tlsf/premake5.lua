project "tlsf"
    kind "StaticLib"
    language "C"
    location "Intermediate"
    targetdir "bin/%{cfg.buildcfg}/%{prj.name}"
    objdir "bin-obj/%{cfg.buildcfg}/%{prj.name}"

    files 
    {
        "include/**.h",
        "src/**.c"
    }

    includedirs
    {
        "include",
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

    filter "configurations:Development*"
		runtime "Release"
		symbols "on"
        optimize "on"
        targetsuffix "_dev"

	filter "configurations:Release*"
		runtime "Release"
		optimize "on"
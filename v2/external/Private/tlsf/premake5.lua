project "tlsf"
    kind "StaticLib"
    language "C"
    location "Intermediate"
    targetdir(tdir)
    objdir(odir)

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
    filter {}

	filter "system:linux"
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
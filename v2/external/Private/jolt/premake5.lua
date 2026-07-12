project "jolt"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"
    location "Intermediate"
    targetdir(tdir)
    objdir(odir)

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
    filter {}

	filter "system:linux or macosx or emscripten"
		pic "On"
        systemversion "latest"
    filter {}

	filter "configurations:Debug*"
		runtime "Debug"
		symbols "on"
        targetsuffix "_d"
        defines
        {
            "JPH_DEBUG_RENDERER"
        }
    filter {}

    filter "configurations:Development*"
		runtime "Release"
		symbols "on"
        optimize "on"
        targetsuffix "_dev"
        defines
        {
            "JPH_DEBUG_RENDERER"
        }
    filter {}

	filter "configurations:Release*"
		runtime "Release"
		optimize "on"
    filter {}
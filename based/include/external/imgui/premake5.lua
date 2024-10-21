project "imgui"
    kind "StaticLib"
    language "C++"
    staticruntime "on"

    targetdir ("bin/%{cfg.buildcfg}/%{prj.name}")
    objdir ("bin-obj/%{cfg.buildcfg}/%{prj.name}")

    files 
    {
        "**.h",
        "**.cpp"
    }

    includedirs
    {
        "%{wks.location}/external/sdl2/include",
        "%{wks.location}/external/glad/include"
    }

    links
    {
        "sdl2",
        "glad"
    }

    flags "NoPCH"
    
    filter "system:windows"
		systemversion "latest"
		cppdialect "C++17"

	filter "system:linux"
		pic "On"
		systemversion "latest"
		cppdialect "C++17"

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"
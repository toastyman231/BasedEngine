project "jolt"
    kind "StaticLib"
    language "C++"
    if externalBuild then location "%{wks.location}/ProjectFiles" end

    if externalBuild then targetdir("%{wks.location}/ProjectFiles/" .. tdir) else targetdir(tdir) end
    if externalBuild then objdir("%{wks.location}/ProjectFiles/" .. odir) else objdir(odir) end

    files 
    {
        "include/**.h",
        "include/**.cpp"
    }

    includedirs
    {
        "include"
    }
    
    flags "NoPCH"
    
    filter "system:windows"
		systemversion "latest"
		cppdialect "C++17"
        staticruntime "on"

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
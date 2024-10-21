project "glad"
    kind "StaticLib"
    language "C"
    staticruntime "on"
    if externalBuild then location "%{wks.location}/ProjectFiles" end

    if externalBuild then targetdir("%{wks.location}/ProjectFiles/" .. tdir) else targetdir(tdir) end
    if externalBuild then objdir("%{wks.location}/ProjectFiles/" .. odir) else objdir(odir) end

    files 
    {
        "include/**.h",
        "src/**.c"
    }

    externalincludedirs
    {
        "include"
    }

    filter {"system:windows"}
        systemversion "latest"

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
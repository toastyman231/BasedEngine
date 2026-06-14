project "assimp"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"
    location "Intermediate"
    targetdir "bin/%{cfg.buildcfg}/%{prj.name}"
    objdir "bin-obj/%{cfg.buildcfg}/%{prj.name}"

    files
    {
        "include/**.h",
        "include/**.hpp",
        "code/**.h",
        "code/**.hpp",
        "code/**.cpp",
        "contrib/pugixml/src/pugixml.cpp",
        "contrib/unzip/**.c",
        "contrib/unzip/**.h",
        "contrib/zlib/**.c",
        "contrib/zlib/**.h",
    }

    removefiles
    {
        -- M3D is disabled by default
        "code/**/M3DImporter*",
        "code/**/M3DExporter*",
        -- USD is disabled by default
        "code/**/USDImporter*",
        "code/**/USD*",
        "code/**/usd*",
        "code/**/USDLoaderImplTinyusdz*",
        -- VRML is disabled by default
        "code/**/VRMLImporter*",
        -- C4D is non-free, disabled
        "code/**/C4DImporter*",
        -- Android JNI
        "code/**/AndroidJNIIOSystem*",
        -- Don't need IFC
        "code/**/IFC*",
        "code/**/ifc*",
        -- We are not compiling for x86
        "contrib/zlib/contrib/inflate86/inffas86.c",
    }

    includedirs
    {
        ".",
        "include",
        "code",
        "contrib",
        "contrib/pugixml/src",
        "contrib/pugixml/contrib",
        "contrib/rapidjson/include",
        "contrib/unzip",
        "contrib/zlib",
        "contrib/utf8cpp/source",
        "contrib/openddlparser/include",
    }

    defines
    {
        "ASSIMP_BUILD_NO_OWN_ZLIB",         -- we're building zlib from contrib ourselves
        "ASSIMP_BUILD_NO_M3D_IMPORTER",
        "ASSIMP_BUILD_NO_M3D_EXPORTER",
        "ASSIMP_BUILD_NO_VRML_IMPORTER",
        "ASSIMP_BUILD_NO_C4D_IMPORTER",
        "ASSIMP_BUILD_NO_USD_IMPORTER",
        "WIN32_LEAN_AND_MEAN",
    }

    enablepch "off"

    filter "system:windows"
        systemversion "latest"
        staticruntime "on"
        linkoptions { "/ignore:4006" }
        defines
        {
            "_CRT_SECURE_NO_WARNINGS",
            "_SCL_SECURE_NO_WARNINGS",
            "UNICODE",
            "_UNICODE",
            "RAPIDJSON_HAS_STDSTRING=1",
            "RAPIDJSON_NOMEMBERITERATORCLASS",
        }

    filter "system:linux"
        pic "On"
        systemversion "latest"
        defines
        {
            "_FILE_OFFSET_BITS=64",
        }

    filter "system:macosx"
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
include "utils.lua"

ENGINE_DIR, SUCCESS, VAL = Utils.GetEngineInstallDir()
if not SUCCESS then print("Error getting engine directory: " .. VAL) end

project "Sandbox"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"

    location "Intermediate"
    targetdir "bin/%{cfg.buildcfg}/%{prj.name}"
    objdir "bin-obj/%{cfg.buildcfg}/%{prj.name}"

    links
    {
        "based",
        --[[ "SDL2",
        "freetype",
        "assimp-vc143-mt",
        "rmlui",
        "rmlui_debugger",
        "ktx", ]]
    }

    files 
    {
        "src/**.h",
        "src/**.cpp"
    }

    includedirs
    {
        "../based/include",
        --[[ "%{externals.spdlog}",
        "%{externals.rmlui}",
        "%{externals.tracy}",
        "%{externals.yaml_cpp}",
        "%{externals.jolt}" ]]
    }

    libdirs
    {
        --[[ "%{libraries.sdl2}",
        "%{libraries.freetype}",
        "%{libraries.assimp}",
        "%{libraries.rmlui}",
        "%{libraries.ktx_software}" ]]
    }

    defines
    {
        "MI_SHARED_LIB",
    }

    fatalwarnings "All"

    postbuildcommands
    {
        "python3 " .. _MAIN_SCRIPT_DIR .. "/tools/basedbuildtool.py -i " .. path.getabsolute("../%{prj.name}") .. " -c %{cfg.buildcfg} -q 0.05 --ci" 
    }

    filter {"system:windows", "configurations:*"}
        systemversion "latest"
        staticruntime "off" -- MUST BE OFF FOR MIMALLOC-REDIRECT TO WORK!
        --files { "resources.rc", "Assets/**.ico" }
        --vpaths { ['Assets/*'] = { '*.rc', '**.ico' } }
        debugdir("bin/%{cfg.buildcfg}/%{prj.name}")
        links { "mimalloc-redirect" }
        libdirs { ENGINE_DIR .. "/external/Private/mimalloc/lib/Windows" }

        defines
        {
            "BASED_PLATFORM_WINDOWS",
            "_SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING",
            "JPH_DEBUG_RENDERER"
        }

    filter {"system:windows", "configurations:Release*"}
        kind "WindowedApp"
    filter {"system:macosx", "configurations:Release*"}
        kind "WindowedApp"

    filter {"system:linux", "configurations:*"}
        defines
        {
            "BASED_PLATFORM_LINUX"
        }
    
    filter "configurations:Debug*"
        defines "BASED_CONFIG_DEBUG"
        runtime "Debug"
        editandcontinue "off"
        symbols "on"
        targetsuffix "_d"

    filter "configurations:Development*"
        defines "BASED_CONFIG_DEVELOPMENT"
        runtime "Release"
        editandcontinue "off"
        optimize "on"
        symbols "on"
        targetsuffix "_dev"

    filter "configurations:Release*"
        defines "BASED_CONFIG_RELEASE"
        fatalwarnings "All"
        runtime "Release"
        symbols "off"
        optimize "on"

    filter "configurations:*Editor"
        defines "BASED_CONFIG_EDITOR"